/*  Size Checker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SizeChecker_H
#define PokemonAutomation_PokemonSV_SizeChecker_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class BoxDetector;


class SizeChecker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SizeChecker_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class SizeChecker : public SingleSwitchProgramInstance{
public:
    SizeChecker();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void enter_check_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void exit_check_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context, VideoSnapshot screen);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> BOXES_TO_CHECK;
    EventNotificationOption NOTIFICATION_MARK;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
