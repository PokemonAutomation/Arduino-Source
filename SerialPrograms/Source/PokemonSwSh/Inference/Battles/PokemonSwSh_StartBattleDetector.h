/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartBattleDetector_H
#define PokemonAutomation_PokemonSwSh_StartBattleDetector_H

#include <functional>
#include <chrono>
#include <thread>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



//bool is_dialog_grey(const QImage& image);


#if 0
//  Return false if timed out.
bool wait_for_start_battle(
    ProgramEnvironment& env,
    VideoFeed& feed,
    VideoOverlay& overlay,
    std::chrono::milliseconds timeout
);
#endif



class StartBattleDetector : public VisualInferenceCallback{
public:
    StartBattleDetector(VideoOverlay& overlay);

    bool detect(const QImage& frame);

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    ImageFloatBox m_screen_box;
    BattleDialogDetector m_dialog;
};







}
}
}
#endif

