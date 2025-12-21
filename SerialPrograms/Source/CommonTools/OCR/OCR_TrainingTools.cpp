/*  Training Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDirIterator>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ComputationThreadPool.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "OCR_SmallDictionaryMatcher.h"
#include "OCR_LargeDictionaryMatcher.h"
#include "OCR_TrainingTools.h"

namespace PokemonAutomation{
namespace OCR{



std::string extract_name(const std::string& filename){
    std::string name = filename;
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
    Logger& logger, CancellableScope& scope,
    const std::string& training_data_directory
)
    : m_logger(logger)
    , m_scope(scope)
    , m_directory(TRAINING_PATH() + training_data_directory)
    , m_total_samples(0)
{
    if (!m_directory.empty() && m_directory.back() != '/' && m_directory.back() != '\\'){
        m_directory += "/";
    }

    logger.log("Parsing training data in: " + m_directory);

    QDirIterator iter(QString::fromStdString(m_directory), QDir::AllDirs);
    while (iter.hasNext()){
        iter.next();
        QString sample_directory = iter.fileName() + "/";
        for (size_t c = 1; c < (size_t)Language::EndOfList; c++){
            Language language = (Language)c;
            const std::string& code = language_data(language).code;
            QString folder = sample_directory + QString::fromStdString(code) + "/";
            QDirIterator iter1(QString::fromStdString(m_directory) + folder, QStringList() << "*.png", QDir::Files);
            while (iter1.hasNext()){
                iter1.next();
//                QString file = iter.next();
                m_samples[language].emplace_back(
                    TrainingSample{
                        OCR::extract_name(iter1.fileName().toStdString()),
                        (folder + iter1.fileName()).toStdString()
                    }
                );
                m_total_samples++;

                scope.throw_if_cancelled();
            }
        }
        scope.throw_if_cancelled();
    }

    logger.log(
        "Parsing Complete: Languages = " + tostr_u_commas(m_samples.size()) +
        ", Samples = " + tostr_u_commas(m_total_samples)
    );
}


void TrainingSession::generate_small_dictionary(
    const std::string& ocr_json_file,
    const std::string& output_json_file,
    bool incremental,
    size_t threads,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double max_log10p, double log10p_spread,
    double min_text_ratio, double max_text_ratio
){
    m_logger.log("Generating OCR Data...");

    OCR::SmallDictionaryMatcher baseline(ocr_json_file, !incremental);
    OCR::SmallDictionaryMatcher trained(ocr_json_file, !incremental);

    ComputationThreadPool task_runner(
        [&](){ GlobalSettings::instance().PERFORMANCE->COMPUTE_PRIORITY.set_on_this_thread(m_logger); },
        0, threads
    );

    std::atomic<size_t> matched(0);
    std::atomic<size_t> failed(0);
    for (const auto& language : m_samples){
        const LanguageData& language_info = language_data(language.first);
        m_logger.log("Starting Language: " + language_info.name);
//        cout << (int)item.first << " : " << item.second.size() << endl;

        task_runner.run_in_parallel(
            [&](size_t index){
                m_scope.throw_if_cancelled();

                const TrainingSample& sample = language.second[index];

                ImageRGB32 image(m_directory + sample.filepath);
                if (!image){
                    m_logger.log("Skipping: " + sample.filepath);
                    return;
                }

                OCR::StringMatchResult result = baseline.match_substring_from_image_multifiltered(
                    nullptr, language.first, image,
                    text_color_ranges,
                    0, log10p_spread,
                    min_text_ratio, max_text_ratio
                );

                OCR::StringMatchResult result0 = result;
                result.clear_beyond_log10p(max_log10p);

                if (result.results.empty()){
                    failed++;
                    result0.log(m_logger, max_log10p, sample.filepath);
                    if (!result0.results.empty()){
                        trained.add_candidate(
                            language.first, sample.token,
                            result0.results.begin()->second.normalized_text
                        );
                    }
                    return;
                }

                for (const auto& item : result.results){
                    if (item.second.token == sample.token){
                        matched++;
                        return;
                    }
                }

                result.log(m_logger, -99999, sample.filepath);
                trained.add_candidate(
                    language.first, sample.token,
                    result0.results.begin()->second.normalized_text
                );
            },
            0, language.second.size(), 1
        );

        m_scope.throw_if_cancelled();
    }

    m_logger.log("Languages: " + tostr_u_commas(m_samples.size()));
    m_logger.log("Samples: " + tostr_u_commas(m_total_samples));
    m_logger.log("Matched: " + tostr_u_commas(matched));
    m_logger.log("Missed: " + tostr_u_commas(failed));

    trained.save(output_json_file);
}

void TrainingSession::generate_large_dictionary(
    const std::string& ocr_json_directory,
    const std::string& output_prefix,
    bool incremental,
    size_t threads,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double max_log10p, double log10p_spread,
    double min_text_ratio, double max_text_ratio
) const{
    m_logger.log("Generating OCR Data...");

    OCR::LargeDictionaryMatcher baseline(ocr_json_directory + output_prefix, nullptr, !incremental);
    OCR::LargeDictionaryMatcher trained(ocr_json_directory + output_prefix, nullptr, !incremental);

    ComputationThreadPool task_runner(
        [&](){ GlobalSettings::instance().PERFORMANCE->COMPUTE_PRIORITY.set_on_this_thread(m_logger); },
        0, threads
    );

    std::atomic<size_t> matched(0);
    std::atomic<size_t> failed(0);
    for (const auto& language : m_samples){
        const LanguageData& language_info = language_data(language.first);
        m_logger.log("Starting Language: " + language_info.name);
//        cout << (int)item.first << " : " << item.second.size() << endl;

        task_runner.run_in_parallel(
            [&](size_t index){
                m_scope.throw_if_cancelled();

                const TrainingSample& sample = language.second[index];

                ImageRGB32 image(m_directory + sample.filepath);
                if (!image){
                    m_logger.log("Skipping: " + sample.filepath);
                    return;
                }

                OCR::StringMatchResult result = baseline.match_substring_from_image_multifiltered(
                    nullptr, language.first, image,
                    text_color_ranges,
                    0, log10p_spread,
                    min_text_ratio, max_text_ratio
                );

                OCR::StringMatchResult result0 = result;
                result.clear_beyond_log10p(max_log10p);

                if (result.results.empty()){
                    failed++;
                    result0.log(m_logger, max_log10p, sample.filepath);
                    if (!result0.results.empty()){
                        trained.add_candidate(
                            language.first, sample.token,
                            result0.results.begin()->second.normalized_text
                        );
                    }
                    return;
                }

                for (const auto& item : result.results){
                    if (item.second.token == sample.token){
                        matched++;
                        return;
                    }
                }

                result.log(m_logger, -99999, sample.filepath);
                trained.add_candidate(
                    language.first, sample.token,
                    result0.results.begin()->second.normalized_text
                );
            },
            0, language.second.size(), 1
        );

        std::string json = output_prefix + language_info.code + ".json";
        trained.save(language.first, json);
        m_scope.throw_if_cancelled();
    }

    m_logger.log("Languages: " + tostr_u_commas(m_samples.size()));
    m_logger.log("Samples: " + tostr_u_commas(m_total_samples));
    m_logger.log("Matched: " + tostr_u_commas(matched));
    m_logger.log("Missed: " + tostr_u_commas(failed));
}






}
}

