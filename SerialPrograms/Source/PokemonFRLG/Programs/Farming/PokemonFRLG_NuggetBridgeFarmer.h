/*  Nugget Bridge Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_NuggetBridgeFarmer_H
#define PokemonAutomation_PokemonFRLG_NuggetBridgeFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class NuggetBridgeFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
	NuggetBridgeFarmer_Descriptor();
	struct Stats;
	virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class NuggetBridgeFarmer : public SingleSwitchProgramInstance {
public:
	NuggetBridgeFarmer();
	virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
	virtual void start_program_border_check(
		VideoStream& stream,
		FeedbackType feedback_type
	) override {
	}

private:

	DeferredStopButtonOption STOP_AFTER_CURRENT;
	SimpleIntegerOption<uint32_t> NUM_NUGGETS;
	GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

	//SimpleIntegerOption<uint32_t> PERIODIC_SAVE;

	EventNotificationOption NOTIFICATION_STATUS_UPDATE;
	EventNotificationsOption NOTIFICATIONS;

};

}
}
}
	 



#endif