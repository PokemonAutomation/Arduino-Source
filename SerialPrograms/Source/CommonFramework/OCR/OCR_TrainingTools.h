/*  Training Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_TrainingTools_H
#define PokemonAutomation_OCR_TrainingTools_H

#include <string>
#include <vector>
#include <map>
#include <QString>
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/LoggerQt.h"

namespace PokemonAutomation{
namespace OCR{


struct TrainingSample{
    std::string token;
    QString filepath;
};


class TrainingSession{
    static constexpr double MAX_LOG10P = -1.90;
    static constexpr double LOG10P_SPREAD = 0.10;

public:
    TrainingSession(
        LoggerQt& logger, CancellableScope& scope,
        const QString& training_data_directory
    );

    void generate_small_dictionary(
        const QString& ocr_json_file,
        const QString& output_json_file,
        bool incremental,
        size_t threads
    );
    void generate_large_dictionary(
        const QString& ocr_json_directory,
        const QString& output_prefix,
        bool incremental,
        size_t threads
    ) const;

private:
    LoggerQt& m_logger;
    CancellableScope& m_scope;
    QString m_directory;
    size_t m_total_samples;
    std::map<Language, std::vector<TrainingSample>> m_samples;
};




std::string extract_name(const QString& filename);


}
}
#endif
