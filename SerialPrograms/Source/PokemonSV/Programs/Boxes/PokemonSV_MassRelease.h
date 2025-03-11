/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MassRelease_H
#define PokemonAutomation_PokemonSV_MassRelease_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class BoxDetector;


class MassRelease_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MassRelease_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class MassRelease : public SingleSwitchProgramInstance{
public:
    MassRelease();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void release_one(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void release_box(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> BOXES_TO_RELEASE;
    BooleanCheckBoxOption SKIP_SHINIES;
    EventNotificationsOption NOTIFICATIONS;

    // Whether the box system view is judege or stats.
    bool m_in_judge_view = false;
};



}
}
}
#endif
