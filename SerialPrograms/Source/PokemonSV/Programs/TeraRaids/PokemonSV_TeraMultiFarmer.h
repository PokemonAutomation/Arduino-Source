/*  Tera Multi-Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraMultiFarmer_H
#define PokemonAutomation_PokemonSV_TeraMultiFarmer_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class PerConsoleTeraFarmerOptions : public GroupOption, public ConfigOption::Listener{
public:
    ~PerConsoleTeraFarmerOptions();
    PerConsoleTeraFarmerOptions(std::string label, const LanguageSet& languages, bool host);

    void set_host(bool is_host);
    virtual void value_changed() override;

public:
    bool is_host;
    StaticTextOption is_host_label;
    OCR::LanguageOCROption language;

    BooleanCheckBoxOption catch_on_win;
    PokemonSwSh::PokemonBallSelectOption ball_select;

    TeraAIOption battle_ai;
};






class TeraMultiFarmer_Descriptor : public MultiSwitchProgramDescriptor{
public:
    TeraMultiFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class TeraMultiFarmer : public MultiSwitchProgramInstance, private ConfigOption::Listener{
public:
    ~TeraMultiFarmer();
    TeraMultiFarmer();
    virtual void update_active_consoles(size_t switch_count) override;
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

    virtual void value_changed() override;

private:
    void reset_host(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);
    void reset_joiner(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

    bool run_raid_host(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);
    void run_raid_joiner(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);

    bool run_raid(MultiSwitchProgramEnvironment& env, CancellableScope& scope);

private:
    enum class Mode{
        FARM_ALONE,
        HOST_LOCALLY,
        HOST_ONLINE,
    };
    EnumDropdownOption<Mode> HOSTING_MODE;

    IntegerEnumDropdownOption HOSTING_SWITCH;
    SimpleIntegerOption<uint16_t> MAX_WINS;
    std::unique_ptr<PerConsoleTeraFarmerOptions> PLAYERS[4];

    enum class RecoveryMode{
        STOP_ON_ERROR,
        SAVE_AND_RESET,
    };
    EnumDropdownOption<RecoveryMode> RECOVERY_MODE;
    BooleanCheckBoxOption ROLLOVER_PREVENTION;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    WallClock m_last_time_fix;
    bool m_errored[4];
};




}
}
}
#endif
