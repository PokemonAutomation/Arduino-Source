/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <memory>
#include <deque>
#include <QFile>
#include <iostream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Globals.h"
#include "TesseractPA.h"
#include "OCR_RawOCR.h"

namespace PokemonAutomation{
namespace OCR{




bool language_available(Language language){
    QString path = RESOURCE_PATH();
    path += "Tesseract/";
    path += QString::fromStdString(language_data(language).code);
    path += ".traineddata";
    QFile file(path);
    return file.exists();
}



class TesseractPool{
public:
    TesseractPool(Language language)
        : m_language_code(language_data(language).code)
        , m_training_data_path(
            RESOURCE_PATH().toStdString() + "Tesseract/"
        )
    {}

    QString run(const ConstImageRef& image){
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

            //  Make sure training data exists.
            std::string path = m_training_data_path + m_language_code + ".traineddata";
            QFile file(QString::fromStdString(path));
            if (!file.exists()){
                return QString();
            }

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
            ? QString()
            : QString::fromUtf8(str.c_str());
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


QString ocr_read(Language language, const ConstImageRef& image){
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




