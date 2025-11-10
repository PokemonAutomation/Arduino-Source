/*  Friendship Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_FriendshipFarmer_H
#define PokemonAutomation_PokemonLZA_FriendshipFarmer_H


#include <memory>
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA {

class FriendshipFarmer_Descriptor : public SingleSwitchProgramDescriptor {
public:
	FriendshipFarmer_Descriptor();

	class Stats;
	virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class FriendshipFarmer : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
	~FriendshipFarmer();
	FriendshipFarmer();

	virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
	void enter_cafe(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
	void exit_bench(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
	void exit_cafe(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
	void hang_out_bench(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

	virtual void on_config_value_changed(void* object) override;

	enum class FarmingOption{
		Cafe,
		Bench
	};
	enum class FriendshipAmount{
		Evolve,
		Max
	};
	EnumDropdownOption<FarmingOption> FARMING_OPTION;
	EnumDropdownOption<FriendshipAmount> FRIENDSHIP_AMOUNT;
	SimpleIntegerOption<uint8_t> NUM_PARTY_MEMBERS;
	GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
	EventNotificationOption NOTIFICATION_STATUS_OPTION;
	EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif