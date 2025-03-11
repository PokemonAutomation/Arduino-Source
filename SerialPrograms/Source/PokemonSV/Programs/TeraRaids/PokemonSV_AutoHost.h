/*  Auto Host
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHost_H
#define PokemonAutomation_PokemonSV_AutoHost_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"
#include "PokemonSV/Options/PokemonSV_AutoHostOptions.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV_JoinTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AutoHost_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoHost_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class AutoHost : public SingleSwitchProgramInstance{
public:
    AutoHost();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    WallClock wait_for_lobby_open(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        std::string& lobby_code
    );
    void update_stats_on_raid_start(SingleSwitchProgramEnvironment& env, uint8_t player_count);
    bool start_raid(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        WallClock start_time,
        uint8_t player_count
    );
    bool run_lobby(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        std::string& lobby_code,
        std::array<std::map<Language, std::string>, 4>& player_names
    );
    void check_kill_switch(ProgramEnvironment& env);

private:
//    OCR::LanguageOCR LANGUAGE;

    EnumDropdownOption<HostingMode> MODE;

    LobbyWaitDelay LOBBY_WAIT_DELAY;
    StartRaidPlayers START_RAID_PLAYERS;
    ShowRaidCode SHOW_RAID_CODE;
    AutoHostDescription DESCRIPTION;
    RemoteKillSwitch REMOTE_KILL_SWITCH0;
    ConsecutiveFailurePause CONSECUTIVE_FAILURE_PAUSE;
    FailurePauseMinutes FAILURE_PAUSE_MINUTES;

    RolloverPrevention ROLLOVER_PREVENTION;
    TeraAIOption BATTLE_AI;

    RaidPlayerBanList BAN_LIST;
    RaidJoinReportOption JOIN_REPORT;

    RaidPostNotification NOTIFICATION_RAID_POST;
    RaidStartNotification NOTIFICATION_RAID_START;
    JoinReportNotification NOTIFICATION_JOIN_REPORT;
    EventNotificationsOption NOTIFICATIONS0;

    WallClock m_killswitch_time;
    std::string m_killswitch_reason;
};




}
}
}
#endif
