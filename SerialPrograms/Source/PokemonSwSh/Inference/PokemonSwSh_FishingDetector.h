/*  Fishing Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FishingDetector_H
#define PokemonAutomation_PokemonSwSh_FishingDetector_H

#include <chrono>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "PokemonSwSh_BattleMenuDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FishingDetector{
public:
    enum Detection{
        NO_DETECTION,
        HOOKED,
        MISSED,
        BATTLE_MENU,
    };

public:
    FishingDetector(VideoFeed& feed);

    Detection detect_now();
    Detection wait_for_detection(
        ProgramEnvironment& env, Logger& logger,
        std::chrono::seconds timeout = std::chrono::seconds(12)
    );

private:
    VideoFeed& m_feed;
    InferenceBoxScope m_hook_box;
    InferenceBoxScope m_miss_box;
    StandardBattleMenuDetector m_battle_menu;
};


}
}
}
#endif
