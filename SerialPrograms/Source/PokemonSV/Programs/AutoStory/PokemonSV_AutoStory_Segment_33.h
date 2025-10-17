/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_33_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_33_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_33 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

// start: Beat Alfornada gym challenge. Beat Alfornada gym. At Alfronada Pokecenter.
// end: Beat Clavell. At Academy fly point.
void checkpoint_85(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Beat Clavell. At Academy fly point.
// end: At Pokemon League entrance.
void checkpoint_86(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: At Pokemon League entrance.
// end: Beat Elite Four. At Pokemon League Pokecenter.
void checkpoint_87(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats, Language language, StarterChoice starter_choice);

std::string get_ride_pokemon_name(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language);



}
}
}
#endif
