/*  Max Lair (Standard Mode)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Standard_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Standard_H

#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "Framework/PokemonSwSh_MaxLair_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class MaxLairStandard_Descriptor : public MultiSwitchProgramDescriptor{
public:
    MaxLairStandard_Descriptor();
};


class MaxLairStandard : public MultiSwitchProgramInstance{
public:
    enum class StopCondition{
        STOP_ON_SHINY_LEGENDARY,
        STOP_ON_NOTHING,
    };

public:
    MaxLairStandard(const MaxLairStandard_Descriptor& descriptor);

    virtual QString check_validity() const override;

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env) override;


private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EnumDropdownOption HOST_SWITCH;
    EnumDropdownOption BOSS_SLOT;

    EnumDropdownOption STOP_CONDITION;

    LanguageSet m_languages;
    MaxLairInternal::MaxLairConsoleOptions PLAYER0;
    MaxLairInternal::MaxLairConsoleOptions PLAYER1;
    MaxLairInternal::MaxLairConsoleOptions PLAYER2;
    MaxLairInternal::MaxLairConsoleOptions PLAYER3;

    MaxLairInternal::HostingSettings HOSTING;

    EventNotificationOption NOTIFICATION_NO_SHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
