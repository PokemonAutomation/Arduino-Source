/*  Training Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_TrainingTools_H
#define PokemonAutomation_CommonTools_OCR_TrainingTools_H

#include <string>
#include <vector>
#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/Language.h"
#include "OCR_Routines.h"

namespace PokemonAutomation{
namespace OCR{


struct TrainingSample{
    std::string token;
    std::string filepath;
};


class TrainingSession{
    static constexpr double MAX_LOG10P = -1.90;
    static constexpr double LOG10P_SPREAD = 0.10;

public:
    TrainingSession(
        Logger& logger, CancellableScope& scope,
        const std::string& training_data_directory
    );

    void generate_small_dictionary(
        const std::string& ocr_json_file,
        const std::string& output_json_file,
        bool incremental,
        size_t threads,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double max_log10p, double log10p_spread,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    );
    void generate_large_dictionary(
        const std::string& ocr_json_directory,
        const std::string& output_prefix,
        bool incremental,
        size_t threads,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double max_log10p, double log10p_spread,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    Logger& m_logger;
    CancellableScope& m_scope;
    std::string m_directory;
    size_t m_total_samples;
    std::map<Language, std::vector<TrainingSample>> m_samples;
};




std::string extract_name(const std::string& filename);


}
}
#endif
