/*  Train OCR Mode Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_TrainOCRModeOption_H
#define PokemonAutomation_CommonTools_Options_TrainOCRModeOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{

enum class TrainOCRMode{
    GENERATE_BASELINE,
    START_FRESH,
    INCREMENTAL,
};

class TrainOCRModeOption : public EnumDropdownOption<TrainOCRMode>{
public:
    TrainOCRModeOption()
        : EnumDropdownOption<TrainOCRMode>(
            "<b>Mode:</b>",
            {
                {TrainOCRMode::GENERATE_BASELINE,   "baseline",     "Generate Baseline: Generate baseline data using display names."},
                {TrainOCRMode::START_FRESH,         "start-fresh",  "Start Fresh: Use only baseline strings. (1st candidate of each entry in above path)"},
                {TrainOCRMode::INCREMENTAL,         "incremental",  "Incremental: Build off of the existing training data in the above path."},
            },
            LockMode::LOCK_WHILE_RUNNING,
            TrainOCRMode::GENERATE_BASELINE
        )
    {}
};



}
#endif
