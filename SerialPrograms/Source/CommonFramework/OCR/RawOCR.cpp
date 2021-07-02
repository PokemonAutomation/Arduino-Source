/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <memory>
#include <deque>
#include <QFile>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/PersistentSettings.h"
#include "TesseractPA.h"
#include "RawOCR.h"

namespace PokemonAutomation{
namespace OCR{




bool language_available(Language language){
    QString path = PERSISTENT_SETTINGS().resource_path;
    path += "Tesseract/";
    path += language_data(language).code.c_str();
    path += ".traineddata";
    QFile file(path);
    return file.exists();
}



class TesseractPool{
public:
    TesseractPool(Language language)
        : m_language_code(language_data(language).code)
        , m_training_data_path(
            std::string(PERSISTENT_SETTINGS().resource_path.toUtf8().data()) + "Tesseract/"
        )
    {}

    QString run(const QImage& image){
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
            QFile file(QString::fromUtf8(path.c_str()));
            if (!file.exists()){
                return QString();
            }

            std::unique_ptr<TesseractAPI> api(
                new TesseractAPI(m_training_data_path.c_str(), m_language_code.c_str())
            );
            if (!api->valid()){
                PA_THROW_StringException("Could not initialize TesseractAPI.");
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

//        auto start = std::chrono::system_clock::now();
        TesseractString str = instance->read32(
            image.bits(),
            image.width(),
            image.height(),
            image.bytesPerLine()
        );
//        auto end = std::chrono::system_clock::now();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << endl;

        {
            SpinLockGuard lg(m_lock, "TesseractPool::run()");
            m_idle.emplace_back(instance);
        }

        return str.c_str() == nullptr
            ? QString()
            : QString::fromUtf8(str.c_str());
    }

private:
    const std::string& m_language_code;
    const std::string m_training_data_path;

    SpinLock m_lock;
    std::vector<std::unique_ptr<TesseractAPI>> m_instances;
    std::vector<TesseractAPI*> m_idle;
};

SpinLock ocr_pool_lock;
std::map<Language, TesseractPool> ocr_pool;


QString ocr_read(Language language, const QImage& image){
//    static size_t c = 0;
//    image.save("test-" + QString::number(c++) + ".png");

    const QImage& ready = image.format() == QImage::Format_RGB32
        ? image
        : image.convertToFormat(QImage::Format_RGB32);

    std::map<Language, TesseractPool>::iterator iter;
    {
        SpinLockGuard lg(ocr_pool_lock, "ocr_read()");
        iter = ocr_pool.find(language);
        if (iter == ocr_pool.end()){
            iter = ocr_pool.emplace(language, language).first;
        }
    }
    return iter->second.run(ready);
}





}
}




