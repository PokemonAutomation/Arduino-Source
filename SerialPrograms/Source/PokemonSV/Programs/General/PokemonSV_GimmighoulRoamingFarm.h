/*  Gimmighoul Roaming Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GimmighoulRoamingFarm_H
#define PokemonAutomation_PokemonSwSh_GimmighoulRoamingFarm_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonSV {

            class GimmighoulRoamingFarm_Descriptor : public SingleSwitchProgramDescriptor {
            public:
                GimmighoulRoamingFarm_Descriptor();
            };

            class GimmighoulRoamingFarm : public SingleSwitchProgramInstance {
            public:
                GimmighoulRoamingFarm();

                virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

            private:
                GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
                SimpleIntegerOption<uint32_t> SKIPS;
                BooleanCheckBoxOption FIX_TIME_WHEN_DONE;
                EventNotificationsOption NOTIFICATIONS;
            };

        }
    }
}
#endif



