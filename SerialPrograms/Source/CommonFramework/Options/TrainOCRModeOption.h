/*  Train OCR Mode Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TrainOCRModeOption_H
#define PokemonAutomation_Options_TrainOCRModeOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{

enum class TrainOCRMode{
    StartFresh,
    Incremental,
};

class TrainOCRModeOption : public EnumDropdownOption<TrainOCRMode>{
public:
    TrainOCRModeOption()
        : EnumDropdownOption<TrainOCRMode>(
            "<b>Mode:</b>",
            {
                {TrainOCRMode::StartFresh, "start-fresh", "Start Fresh: Use only baseline strings. (1st candidate of each entry in above path)"},
                {TrainOCRMode::Incremental, "incremental", "Incremental: Build off of the existing training data in the above path."},
            },
            LockWhileRunning::LOCKED,
            TrainOCRMode::StartFresh
        )
    {}
};



}
#endif
