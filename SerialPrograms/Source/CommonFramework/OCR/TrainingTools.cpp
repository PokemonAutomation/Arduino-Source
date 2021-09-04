/*  Training Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDirIterator>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ParallelTaskRunner.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/Filtering.h"
#include "SmallDictionaryMatcher.h"
#include "LargeDictionaryMatcher.h"
#include "TrainingTools.h"

namespace PokemonAutomation{
namespace OCR{



std::string extract_name(const QString& filename){
    std::string name = filename.toUtf8().data();
    while (!name.empty()){
        char ch = name.back();
        name.pop_back();
        if (ch == '.'){
            break;
        }
    }
    while (!name.empty()){
        char ch = name.back();
        name.pop_back();
        if (ch == '-'){
            break;
        }
    }
    while (!name.empty()){
        char ch = name.back();
        name.pop_back();
        if (ch == '-'){
            break;
        }
    }
    return name;
}



TrainingSession::TrainingSession(
    ProgramEnvironment& env,
    const QString& training_data_directory
)
    : m_directory(PERSISTENT_SETTINGS().training_data + training_data_directory)
    , m_total_samples(0)
{
    if (!m_directory.isEmpty() && m_directory.back() != '/' && m_directory.back() != '\\'){
        m_directory += "/";
    }

    env.log("Parsing training data in: " + m_directory);

    QDirIterator iter(m_directory, QDir::AllDirs);
    while (iter.hasNext()){
        iter.next();
        QString sample_directory = iter.fileName() + "/";
        for (size_t c = 1; c < (size_t)Language::EndOfList; c++){
            Language language = (Language)c;
            const std::string& code = language_data(language).code;
            QString folder = sample_directory + QString::fromStdString(code) + "/";
            QDirIterator iter(m_directory + folder, QStringList() << "*.png", QDir::Files);
            while (iter.hasNext()){
                iter.next();
//                QString file = iter.next();
                m_samples[language].emplace_back(
                    TrainingSample{
                        OCR::extract_name(iter.fileName()),
                        folder + iter.fileName()
                    }
                );
                m_total_samples++;

                env.check_stopping();
            }
        }
        env.check_stopping();
    }

    env.log(
        "Parsing Complete: Languages = " + tostr_u_commas(m_samples.size()) +
        ", Samples = " + tostr_u_commas(m_total_samples)
    );
}


void TrainingSession::generate_small_dictionary(
    ProgramEnvironment& env,
    const QString& ocr_json_file,
    const QString& output_json_file,
    bool incremental,
    size_t threads
){
    env.log("Generating OCR Data...");

    OCR::SmallDictionaryMatcher baseline(ocr_json_file, !incremental);
    OCR::SmallDictionaryMatcher trained(ocr_json_file, !incremental);

    ParallelTaskRunner task_runner(0, threads);

    std::atomic<size_t> matched(0);
    std::atomic<size_t> failed(0);
    for (const auto& language : m_samples){
        const LanguageData& language_info = language_data(language.first);
        env.log("Starting Language: " + language_info.name);
//        cout << (int)item.first << " : " << item.second.size() << endl;
        for (const TrainingSample& sample : language.second){
            task_runner.dispatch([&]{
                QImage image(m_directory + sample.filepath);
                if (image.isNull()){
                    env.log("Skipping: " + sample.filepath);
                    return;
                }
                OCR::make_OCR_filter(image).apply(image);
                QString text = OCR::ocr_read(language.first, image);

                OCR::MatchResult result = baseline.match_substring(
                    language.first,
                    sample.token,
                    text
                );
                if (result.matched){
                    matched++;
//                    result.log(&env.logger(), sample.filepath);
                }else{
                    failed++;
                    result.log(env.logger(), sample.filepath);
                    trained.add_candidate(language.first, sample.token, result.normalized_text);
                }

//                cout << "matched = " << matched << ", failed = " << failed << endl;
            });

            env.check_stopping();
        }

        task_runner.wait_for_everything();

        env.check_stopping();
    }

    env.log("Languages: " + tostr_u_commas(m_samples.size()));
    env.log("Samples: " + tostr_u_commas(m_total_samples));
    env.log("Matched: " + tostr_u_commas(matched));
    env.log("Missed: " + tostr_u_commas(failed));

    trained.save(output_json_file);
}

void TrainingSession::generate_large_dictionary(
    ProgramEnvironment& env,
    const QString& ocr_json_directory,
    const QString& output_prefix,
    bool incremental,
    size_t threads
) const{
    env.log("Generating OCR Data...");

    OCR::LargeDictionaryMatcher baseline(ocr_json_directory + output_prefix, !incremental);
    OCR::LargeDictionaryMatcher trained(ocr_json_directory + output_prefix, !incremental);

    ParallelTaskRunner task_runner(0, threads);

    std::atomic<size_t> matched(0);
    std::atomic<size_t> failed(0);
    for (const auto& language : m_samples){
        const LanguageData& language_info = language_data(language.first);
        env.log("Starting Language: " + language_info.name);
//        cout << (int)item.first << " : " << item.second.size() << endl;
        for (const TrainingSample& sample : language.second){
            task_runner.dispatch([&]{
                QImage image(m_directory + sample.filepath);
                if (image.isNull()){
                    env.log("Skipping: " + sample.filepath);
                    return;
                }
                OCR::make_OCR_filter(image).apply(image);
                QString text = OCR::ocr_read(language.first, image);

                OCR::MatchResult result = baseline.match_substring(
                    language.first,
                    sample.token,
                    text
                );
                if (result.matched){
                    matched++;
                }else{
                    failed++;
                    result.log(env.logger(), sample.filepath);
                    trained.add_candidate(language.first, sample.token, result.normalized_text);
                }

            });

            env.check_stopping();
        }

        task_runner.wait_for_everything();

        QString json = output_prefix + QString::fromStdString(language_info.code) + ".json";
        trained.save(language.first, json);

        env.check_stopping();
    }

    env.log("Languages: " + tostr_u_commas(m_samples.size()));
    env.log("Samples: " + tostr_u_commas(m_total_samples));
    env.log("Matched: " + tostr_u_commas(matched));
    env.log("Missed: " + tostr_u_commas(failed));
}






}
}

