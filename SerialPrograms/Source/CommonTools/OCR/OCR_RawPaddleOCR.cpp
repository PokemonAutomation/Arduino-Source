/*  Threadpools for PaddleOCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include <deque>
#include <QFile>
#include <QDir>
#include "ML/Inference/ML_PaddleOCRPipeline.h"
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



// // Thread-safe object pool for PaddleOCR instances for a specific language.
// // Allows concurrent OCR operations by maintaining multiple PaddleOCR instances that can be
// // checked out, used, and returned. Instances are created lazily on demand.
// class PaddlePool{
// public:
//     PaddlePool(Language language)
//         : m_language(language)
//     {}

//     // Perform OCR on the given image. Thread-safe - can be called concurrently.
//     // Checkout pattern: (1) acquire idle instance from pool (or create new one if none available),
//     // (2) run OCR without holding lock, (3) return instance to idle pool.
//     std::string run(const ImageViewRGB32& image){
//         ML::PaddleOCRPipeline* instance = nullptr;
//         // Checkout: Try to get an idle instance, create new one if the idle pool is empty.
//         while (true){
//             {
//                 WriteSpinLock lg(m_lock, "PaddlePool::run()1");
//                 if (!m_idle.empty()){
//                     instance = m_idle.back();
//                     m_idle.pop_back();
//                     break;
//                 }
//             }
//             // No idle instance available - create a new one.
//             add_paddle_instance();
//         }

//         // Perform OCR without holding the lock (allows concurrent OCR operations).
// //        auto start = current_time();
//         std::string str = instance->recognize(image);

// //        auto end = current_time();
// //        cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << endl;
//         // Checkin: Return instance to the idle pool.
//         {
//             WriteSpinLock lg(m_lock, "PaddlePool::run()2");
//             m_idle.emplace_back(instance);
//         }

//         return str.c_str() == nullptr
//             ? std::string()
//             : str.c_str();
//     }

//     // Create a new PaddleOCR instance and add it to the pool.
//     // Thread-safe - can be called concurrently (using `m_lock` when modifying the pool).
//     void add_paddle_instance(){

//         global_logger_tagged().log(
//             "Initializing PaddleOCR (" + language_data(m_language).name + "): "
//         );
//         // Create instance outside lock (initialization is expensive).
//         std::unique_ptr<ML::PaddleOCRPipeline> api = std::make_unique<ML::PaddleOCRPipeline>(m_language);

//         // if (!api->valid()){
//         //     throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Could not initialize PaddleOCR.");
//         // }

//         // Add to pool under lock.
//         WriteSpinLock lg(m_lock, "PaddlePool::add_paddle_instance()");

//         m_instances.emplace_back(std::move(api));
//         try{
//             m_idle.emplace_back(m_instances.back().get());
//         }catch (...){
//             m_instances.pop_back();
//             throw;
//         }
//     }

//     // Pre-allocate a minimum number of instances to avoid lazy initialization during runtime.
//     // Useful for warming up the pool before heavy OCR workloads.
//     void ensure_paddle_instances(size_t instances){
//         size_t current_instances;
//         while (true){
//             {
//                 ReadSpinLock lg(m_lock, "PaddlePool::ensure_paddle_instances()");
//                 current_instances = m_instances.size();
//             }
//             if (current_instances >= instances){
//                 break;
//             }
//             add_paddle_instance();
//         }
//     }

// #if 0
//     #ifdef __APPLE__
//     #ifdef UNIX_LINK_TESSERACT
//         ~PaddlePool(){
//             for(auto& api : m_instances){
//                 api.release();
//             }
//         }
//     #endif
//     #endif
// #endif

// private:
//     const Language m_language;

//     // Concurrency: m_lock protects both m_instances and m_idle vectors.
//     SpinLock m_lock;
//     // Owns all created PaddleOCR instances.
//     std::vector<std::unique_ptr<ML::PaddleOCRPipeline>> m_instances;
//     // Current idle PaddleOCR instances in `m_instances`.
//     std::vector<ML::PaddleOCRPipeline*> m_idle;
// };

enum class LanguageGroup {
    None,
    English,
    ChineseJapanese,
    Latin,
    Korean,
};

LanguageGroup language_to_languagegroup(Language language){
    switch(language){
    case Language::None:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
    case Language::English:
        return LanguageGroup::English;
    case Language::Japanese:
        return LanguageGroup::ChineseJapanese;
    case Language::Spanish:
        return LanguageGroup::Latin;
    case Language::French:
        return LanguageGroup::Latin;
    case Language::German:
        return LanguageGroup::Latin;
    case Language::Italian:
        return LanguageGroup::Latin;
    case Language::Korean:
        return LanguageGroup::Korean;
    case Language::ChineseSimplified:
        return LanguageGroup::ChineseJapanese;
    case Language::ChineseTraditional:
        return LanguageGroup::ChineseJapanese;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR on an unknown language.");
    }
}


// Global singleton managing PaddlePools for all languages.
// Two-level concurrency model:
//   Level 1: ocr_pool_lock protects the map (language -> pool creation/lookup)
//   Level 2: Each PaddlePool has its own m_lock (instance checkout/checkin)
struct PaddleOcrGlobals{
    SpinLock ocr_pool_lock;                       // Protects ocr_pool map.
    std::map<LanguageGroup, ML::PaddleOCRPipeline> ocr_pool;   // One instance per language.

    static PaddleOcrGlobals& instance(){
        static PaddleOcrGlobals globals;
        return globals;
    }
};



std::string paddle_ocr_read(Language language, const ImageViewRGB32& image){
//    static size_t c = 0;
//    image.save("ocr-" + std::to_string(c++) + ".png");

    if (language == Language::None){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
    }

    LanguageGroup language_group = language_to_languagegroup(language);

    PaddleOcrGlobals& globals = PaddleOcrGlobals::instance();
    std::map<LanguageGroup, ML::PaddleOCRPipeline>& ocr_pool = globals.ocr_pool;

    // Get or create the pool for this language (lock only during map access).
    std::map<LanguageGroup, ML::PaddleOCRPipeline>::iterator iter;
    {
        WriteSpinLock lg(globals.ocr_pool_lock, "paddle_ocr_read()");
        iter = ocr_pool.find(language_group);
        if (iter == ocr_pool.end()){
            iter = ocr_pool.emplace(language_group, language).first;
        }
    }
    // Run inference with the paddle model. 
    // PaddleOCR with Onnx is threadsafe, so a single instance can be called by multiple threads.
    std::string ret = iter->second.recognize(image);

//    global_logger_tagged().log(ret);

    return ret;
}


// void ensure_paddle_ocr_instances(Language language, size_t instances){
//     if (language == Language::None){
//         throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
//     }

//     PaddleOcrGlobals& globals = PaddleOcrGlobals::instance();
//     std::map<Language, PaddlePool>& ocr_pool = globals.ocr_pool;

//     // Get or create the pool for this language.
//     std::map<Language, PaddlePool>::iterator iter;
//     {
//         WriteSpinLock lg(globals.ocr_pool_lock, "ensure_paddle_ocr_instances()");
//         iter = ocr_pool.find(language);
//         if (iter == ocr_pool.end()){
//             iter = ocr_pool.emplace(language, language).first;
//         }
//     }
//     // Delegate to pool's ensure_paddle_instances (which handles its own locking).
//     iter->second.ensure_paddle_instances(instances);
// }

// void clear_paddle_ocr_cache(){
//     PaddleOcrGlobals& globals = PaddleOcrGlobals::instance();
//     std::map<Language, PaddlePool>& ocr_pool = globals.ocr_pool;
//     WriteSpinLock lg(globals.ocr_pool_lock, "clear_paddle_ocr_cache()");
//     ocr_pool.clear();  // Destroys all pools and their instances.
// }




}
}




