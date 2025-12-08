/*  Weather Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Reset the weather by siting on a bench until reach the desired value
 */

#ifndef PokemonAutomation_PokemonLZA_WeatherFinder_H
#define PokemonAutomation_PokemonLZA_WeatherFinder_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include <CommonFramework/GlobalSettingsPanel.h>

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonLZA {


            class WeatherFinder_Descriptor : public SingleSwitchProgramDescriptor {
            public:
                WeatherFinder_Descriptor();

                class Stats;
                virtual std::unique_ptr<StatsTracker> make_stats() const override;
            };


            class WeatherFinder : public SingleSwitchProgramInstance {
            public:
                WeatherFinder();

                virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

            private:
                IntegerEnumDropdownOption START_POSITION;
                IntegerEnumDropdownOption DESIRED_WEATHER;
                GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

                EventNotificationOption NOTIFICATION_STATUS;
                EventNotificationsOption NOTIFICATIONS;
            };
}
}
}
#endif
