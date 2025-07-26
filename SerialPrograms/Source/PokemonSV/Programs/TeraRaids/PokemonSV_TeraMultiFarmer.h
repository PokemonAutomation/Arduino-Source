/*  Tera Multi-Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraMultiFarmer_H
#define PokemonAutomation_PokemonSV_TeraMultiFarmer_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"
#include "PokemonSV/Options/PokemonSV_AutoHostOptions.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"
#include "PokemonSV_AutoHostTools.h"
#include "PokemonSV_JoinTracker.h"
#include "PokemonSV_TeraRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class GeneralHostingOptions : public GroupOption{
public:
    GeneralHostingOptions();

    LobbyWaitDelay LOBBY_WAIT_DELAY;
    StartRaidPlayers START_RAID_PLAYERS;
    ShowRaidCode SHOW_RAID_CODE;
    AutoHostDescription DESCRIPTION;
    RemoteKillSwitch REMOTE_KILL_SWITCH;
    ConsecutiveFailurePause CONSECUTIVE_FAILURE_PAUSE;
    FailurePauseMinutes FAILURE_PAUSE_MINUTES;
};



class TeraFarmerPerConsoleOptions : public GroupOption, public ConfigOption::Listener{
public:
    ~TeraFarmerPerConsoleOptions();
    TeraFarmerPerConsoleOptions(std::string label, const LanguageSet& languages, bool host);

    void set_host(bool is_host);
    virtual void on_config_value_changed(void* object) override;

public:
    bool is_host;
    StaticTextOption is_host_label;
    OCR::LanguageOCROption language;
    KeyboardLayoutOption keyboard_layout;

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

    virtual void on_config_value_changed(void* object) override;

private:
    void reset_host(const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context);
    void reset_joiner(const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context);

    bool run_raid_host(ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context);
    void run_raid_joiner(ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context);
    void join_lobby(
        ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
        size_t host_index, const std::string& normalized_code
    );

    bool start_sequence_host(
        MultiSwitchProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
        RaidWaiter& raid_waiter, CancellableScope& joiner_scope,
        std::string& lobby_code,
        std::array<std::map<Language, std::string>, 4>& player_names
    );
    void start_sequence_joiner(
        ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
        RaidWaiter& raid_waiter
    );

    bool run_raid(
        MultiSwitchProgramEnvironment& env, CancellableScope& scope,
        std::string& lobby_code,
        std::array<std::map<Language, std::string>, 4>& player_names
    );

private:
    IntegerEnumDropdownOption HOSTING_SWITCH;
    SimpleIntegerOption<uint16_t> MAX_WINS;

    enum class Mode{
        FARM_ALONE,
        HOST_LOCALLY,
        HOST_ONLINE,
    };
    EnumDropdownOption<Mode> HOSTING_MODE;

    GeneralHostingOptions HOSTING_OPTIONS;

    //  Per-console Options
    std::unique_ptr<TeraFarmerPerConsoleOptions> PLAYERS[4];

    enum class RecoveryMode{
        STOP_ON_ERROR,
        SAVE_AND_RESET,
    };
    EnumDropdownOption<RecoveryMode> RECOVERY_MODE;
    RolloverPrevention ROLLOVER_PREVENTION;

    //  Extended Auto-host Options
    RaidPlayerBanList BAN_LIST;
    RaidJoinReportOption JOIN_REPORT;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    RaidPostNotification NOTIFICATION_RAID_POST;
    RaidStartNotification NOTIFICATION_RAID_START;
    JoinReportNotification NOTIFICATION_JOIN_REPORT;
    EventNotificationsOption NOTIFICATIONS;

    WallClock m_last_time_fix;
//    std::atomic<bool> m_raid_error;
    std::atomic<bool> m_reset_required[4];
};




}
}
}
#endif
