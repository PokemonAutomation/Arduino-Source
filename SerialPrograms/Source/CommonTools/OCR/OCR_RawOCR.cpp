/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include <deque>
#include <QFile>
#include <QDir>
#include "3rdParty/TesseractPA/TesseractPA.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "OCR_RawOCR.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{


bool language_available(Language language){
    std::string path = RESOURCE_PATH();
    path += "Tesseract/";
    path += language_data(language).code;
    path += ".traineddata";
    QFile file(QString::fromStdString(path));
    return file.exists();
}


// Thread-safe object pool for TesseractAPI instances for a specific language.
// Allows concurrent OCR operations by maintaining multiple Tesseract instances that can be
// checked out, used, and returned. Instances are created lazily on demand.
class TesseractPool{
public:
    TesseractPool(Language language)
        : m_language_code(language_data(language).code)
        , m_training_data_path(
            QDir::current().relativeFilePath(QString::fromStdString(RESOURCE_PATH() + "Tesseract/")).toStdString()
        )
    {}

    // Perform OCR on the given image. Thread-safe - can be called concurrently.
    // Checkout pattern: (1) acquire idle instance from pool (or create new one if none available),
    // (2) configure PSM, (3) run OCR without holding lock, (4) return instance to idle pool.
    std::string run(const ImageViewRGB32& image, int psm){
        TesseractAPI* instance;
        // Checkout: Try to get an idle instance, create new one if the idle pool is empty.
        while (true){
            {
                WriteSpinLock lg(m_lock, "TesseractPool::run()");
                if (!m_idle.empty()){
                    instance = m_idle.back();
                    m_idle.pop_back();
                    break;
                }
            }
            // No idle instance available - create a new one.
            add_instance();
        }

        // Configure PSM before OCR (safe to call between images on same instance).
        // PSM is page segmentation mode for Tessearct.
        instance->set_page_seg_mode(psm);

        // Perform OCR without holding the lock (allows concurrent OCR operations).
//        auto start = current_time();
        TesseractString str = instance->read32(
            (const unsigned char*)image.data(),
            image.width(),
            image.height(),
            image.bytes_per_row()
        );
//        auto end = current_time();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << endl;
        // Checkin: Return instance to the idle pool.
        {
            WriteSpinLock lg(m_lock, "TesseractPool::run()");
            m_idle.emplace_back(instance);
        }

        return str.c_str() == nullptr
            ? std::string()
            : str.c_str();
    }

    // Create a new TesseractAPI instance and add it to the pool.
    // Thread-safe - can be called concurrently (using `m_lock` when modifying the pool).
    void add_instance(){
        //  Check for non-ascii characters in path.
        for (char ch : m_training_data_path){
            if (ch < 0){
                throw InternalSystemError(
                    nullptr, PA_CURRENT_FUNCTION,
                    "Detected non-ASCII character in Tesseract path. Please move the program to a path with only ASCII characters."
                );
            }
        }

        global_logger_tagged().log(
            "Initializing TesseractAPI (" + m_language_code + "): " + m_training_data_path
        );
        // Create instance outside lock (initialization is expensive).
        std::unique_ptr<TesseractAPI> api(
            new TesseractAPI(m_training_data_path.c_str(), m_language_code.c_str())
        );
        if (!api->valid()){
            throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Could not initialize TesseractAPI.");
        }

        // Add to pool under lock.
        WriteSpinLock lg(m_lock, "TesseractPool::run()");

        m_instances.emplace_back(std::move(api));
        try{
            m_idle.emplace_back(m_instances.back().get());
        }catch (...){
            m_instances.pop_back();
            throw;
        }
    }

    // Pre-allocate a minimum number of instances to avoid lazy initialization during runtime.
    // Useful for warming up the pool before heavy OCR workloads.
    void ensure_instances(size_t instances){
        size_t current_instances;
        while (true){
            {
                ReadSpinLock lg(m_lock, "TesseractPool::run()");
                current_instances = m_instances.size();
            }
            if (current_instances >= instances){
                break;
            }
            add_instance();
        }
    }

#ifdef __APPLE__
#ifdef UNIX_LINK_TESSERACT
    ~TesseractPool(){
        for(auto& api : m_instances){
            api.release();
        }
    }
#endif
#endif

private:
    const std::string& m_language_code;
    const std::string m_training_data_path;

    // Concurrency: m_lock protects both m_instances and m_idle vectors.
    SpinLock m_lock;
    // Owns all created Tesseract instances.
    std::vector<std::unique_ptr<TesseractAPI>> m_instances;
    // Current idle Tesseract instances in `m_instances`.
    std::vector<TesseractAPI*> m_idle;
};

// Global singleton managing TesseractPools for all languages.
// Two-level concurrency model:
//   Level 1: ocr_pool_lock protects the map (language -> pool creation/lookup)
//   Level 2: Each TesseractPool has its own m_lock (instance checkout/checkin)
struct OcrGlobals{
    SpinLock ocr_pool_lock;                       // Protects ocr_pool map.
    std::map<Language, TesseractPool> ocr_pool;   // One pool per language.

    static OcrGlobals& instance(){
        static OcrGlobals globals;
        return globals;
    }
};


std::string ocr_read(Language language, const ImageViewRGB32& image, PageSegMode psm){
//    static size_t c = 0;
//    image.save("ocr-" + std::to_string(c++) + ".png");

    if (language == Language::None){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
    }

    OcrGlobals& globals = OcrGlobals::instance();
    std::map<Language, TesseractPool>& ocr_pool = globals.ocr_pool;

    // Get or create the pool for this language (lock only during map access).
    std::map<Language, TesseractPool>::iterator iter;
    {
        WriteSpinLock lg(globals.ocr_pool_lock, "ocr_read()");
        iter = ocr_pool.find(language);
        if (iter == ocr_pool.end()){
            iter = ocr_pool.emplace(language, language).first;
        }
    }
    // Delegate to pool (which has its own locking for instance management).
    std::string ret = iter->second.run(image, static_cast<int>(psm));

//    global_logger_tagged().log(ret);

    return ret;
}


void ensure_instances(Language language, size_t instances){
    if (language == Language::None){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
    }

    OcrGlobals& globals = OcrGlobals::instance();
    std::map<Language, TesseractPool>& ocr_pool = globals.ocr_pool;

    // Get or create the pool for this language.
    std::map<Language, TesseractPool>::iterator iter;
    {
        WriteSpinLock lg(globals.ocr_pool_lock, "ocr_read()");
        iter = ocr_pool.find(language);
        if (iter == ocr_pool.end()){
            iter = ocr_pool.emplace(language, language).first;
        }
    }
    // Delegate to pool's ensure_instances (which handles its own locking).
    iter->second.ensure_instances(instances);
}

void clear_cache(){
    OcrGlobals& globals = OcrGlobals::instance();
    std::map<Language, TesseractPool>& ocr_pool = globals.ocr_pool;
    WriteSpinLock lg(globals.ocr_pool_lock, "ocr_clear_cache()");
    ocr_pool.clear();  // Destroys all pools and their instances.
}




}
}




