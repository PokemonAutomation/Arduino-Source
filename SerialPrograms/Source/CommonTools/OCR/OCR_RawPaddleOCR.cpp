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
    std::map<LanguageGroup, std::shared_ptr<ML::PaddleOCRPipeline>> ocr_pool;   // One instance per language.

    static PaddleOcrGlobals& instance(){
        static PaddleOcrGlobals globals;
        return globals;
    }
};

std::shared_ptr<ML::PaddleOCRPipeline> ensure_paddle_ocr_instance(Language language){
    if (language == Language::None){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
    }

    LanguageGroup language_group = language_to_languagegroup(language);

    PaddleOcrGlobals& globals = PaddleOcrGlobals::instance();
    std::map<LanguageGroup, std::shared_ptr<ML::PaddleOCRPipeline>>& ocr_pool = globals.ocr_pool;

    // Get or create the Paddle instance for this language.
    std::map<LanguageGroup, std::shared_ptr<ML::PaddleOCRPipeline>>::iterator iter;
    {
        WriteSpinLock lg(globals.ocr_pool_lock, "ensure_paddle_ocr_instances()");
        // std::lock_guard<std::mutex> lg(globals.ocr_pool_lock);
        iter = ocr_pool.find(language_group);
        if (iter == ocr_pool.end()){
            iter = ocr_pool.try_emplace(language_group, std::make_shared<ML::PaddleOCRPipeline>(language)).first;
        }
    }

    return iter->second;
}


std::string paddle_ocr_read(Language language, const ImageViewRGB32& image){
//    static size_t c = 0;
//    image.save("ocr-" + std::to_string(c++) + ".png");

    std::shared_ptr<ML::PaddleOCRPipeline> paddle_instance = ensure_paddle_ocr_instance(language);
    
    // Run inference with the paddle model. 
    // PaddleOCR with Onnx is threadsafe, so a single instance can be called by multiple threads.
    std::string ret = paddle_instance->recognize(image);

//    global_logger_tagged().log(ret);

    return ret;
}




void clear_paddle_ocr_cache(){
    PaddleOcrGlobals& globals = PaddleOcrGlobals::instance();
    std::map<LanguageGroup, std::shared_ptr<ML::PaddleOCRPipeline>>& ocr_pool = globals.ocr_pool;
    WriteSpinLock lg(globals.ocr_pool_lock, "clear_paddle_ocr_cache()");
    // std::lock_guard<std::mutex> lg(globals.ocr_pool_lock);
    ocr_pool.clear();  // Destroys all pools and their instances.
}




}
}




