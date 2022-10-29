/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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



class TesseractPool{
public:
    TesseractPool(Language language)
        : m_language_code(language_data(language).code)
        , m_training_data_path(
            QDir::current().relativeFilePath(QString::fromStdString(RESOURCE_PATH() + "Tesseract/")).toStdString()
        )
    {}

    std::string run(const ImageViewRGB32& image){
        TesseractAPI* instance;
        do{
            {
                SpinLockGuard lg(m_lock, "TesseractPool::run()");
                if (!m_idle.empty()){
                    instance = m_idle.back();
                    m_idle.pop_back();
                    break;
                }
            }

#if 0
            //  Make sure training data exists.
            std::string path = m_training_data_path + m_language_code + ".traineddata";
            QFile file(QString::fromStdString(path));
            if (!file.exists()){
                return QString();
            }
#endif

//            cout << m_training_data_path << endl;

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
            std::unique_ptr<TesseractAPI> api(
                new TesseractAPI(m_training_data_path.c_str(), m_language_code.c_str())
            );
            if (!api->valid()){
                throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Could not initialize TesseractAPI.");
            }

            SpinLockGuard lg(m_lock, "TesseractPool::run()");

            m_instances.emplace_back(std::move(api));
            try{
                m_idle.emplace_back(m_instances.back().get());
            }catch (...){
                m_instances.pop_back();
                throw;
            }
            instance = m_idle.back();
            m_idle.pop_back();
        }while (false);

//        auto start = current_time();
        TesseractString str = instance->read32(
            (const unsigned char*)image.data(),
            image.width(),
            image.height(),
            image.bytes_per_row()
        );
//        auto end = current_time();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << endl;

        {
            SpinLockGuard lg(m_lock, "TesseractPool::run()");
            m_idle.emplace_back(instance);
        }

        return str.c_str() == nullptr
            ? std::string()
            : str.c_str();
    }

#ifdef __APPLE__
#ifdef UNIX_LINK_TESSERACT
    ~TesseractPool(){
        // As of Feb 05, 2022, the newest Tesseract (5.0.1) installed by HomeBrew on macOS
        // has a bug that will crash the program when deleting internal Tesseract API intances,
        // giving error: 
        // libc++abi.dylib: terminating with uncaught exception of type std::__1::system_error: mutex lock failed: Invalid argument
        // A similar issue is posted on Tesseract Github: https://github.com/tesseract-ocr/tesseract/issues/3655
        // There is no way of using HomeBrew to reinstall the older version.
        // Fortunately this class TesseractPool will not get built and destroyed repeatedly in
        // runtime. It will only get initialized once for each supported language. So I am able
        // to use this ugly workaround by not deleting the Tesseract API intances.
        std::cout << "Warning: not release Tesseract API istance due to mutex bug similar to https://github.com/tesseract-ocr/tesseract/issues/3655" << std::endl;
        for(auto& api : m_instances){
            api.release();
        }
    }
#endif
#endif

private:
    const std::string& m_language_code;
    const std::string m_training_data_path;

    SpinLock m_lock;
    std::vector<std::unique_ptr<TesseractAPI>> m_instances;
    std::vector<TesseractAPI*> m_idle;
};

SpinLock ocr_pool_lock;
std::map<Language, TesseractPool> ocr_pool;


std::string ocr_read(Language language, const ImageViewRGB32& image){
//    static size_t c = 0;
//    image.save("test-" + QString::number(c++) + ".png");

    std::map<Language, TesseractPool>::iterator iter;
    {
        SpinLockGuard lg(ocr_pool_lock, "ocr_read()");
        iter = ocr_pool.find(language);
        if (iter == ocr_pool.end()){
            iter = ocr_pool.emplace(language, language).first;
        }
    }
    return iter->second.run(image);
}





}
}




