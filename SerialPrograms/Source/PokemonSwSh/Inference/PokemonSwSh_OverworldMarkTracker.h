/*  Overworld Mark Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldMarkTracker_H
#define PokemonAutomation_PokemonSwSh_OverworldMarkTracker_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class OverworldMarkTracker{
public:
    OverworldMarkTracker(ProgramEnvironment& env, VideoFeed& feed);



private:
    ProgramEnvironment& m_env;
    VideoFeed& m_feed;


};



}
}
}
#endif
