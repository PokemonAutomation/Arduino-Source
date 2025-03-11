/*  Auto Host Lobby Waiter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHosLobbyWaiter_H
#define PokemonAutomation_PokemonSV_AutoHosLobbyWaiter_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV_JoinTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class TeraLobbyWaiter{
public:
    TeraLobbyWaiter(
        ProgramEnvironment& env,
        VideoStream& stream, ProControllerContext& context,
        uint8_t host_players,
        const std::string& lobby_code, WallClock start_time,
        SimpleIntegerOption<uint16_t>& LOBBY_WAIT_DELAY,
        IntegerEnumDropdownOption& START_RAID_PLAYERS,
        EventNotificationOption& NOTIFICATION_RAID_START,
        RaidPlayerBanList& BAN_LIST,
        RaidJoinReportOption& JOIN_REPORT
    );

    uint8_t last_known_players(){
        return m_total_players;
    }
    const std::array<std::map<Language, std::string>, 4>& names(){
        return m_names;
    }

    enum class LobbyResult{
        RAID_STARTED,
        RAID_FAILED,
        BANNED_PLAYER,
    };
    LobbyResult run_lobby();


private:
    VideoSnapshot synchronize_state();

    std::string check_hat_trick();
    bool check_bans(bool skip_grace_period);
    bool check_start_timeout();
    bool check_enough_players();


    bool process_hat_trick(const ImageViewRGB32& snapshot);
    bool process_bans(const ImageViewRGB32& snapshot, bool skip_grace_period);
    bool process_start_timeout(const ImageViewRGB32& snapshot);
    bool process_enough_players(const ImageViewRGB32& snapshot);



private:
    ProgramEnvironment& m_env;
    VideoStream& m_stream;
    ProControllerContext& m_context;
    uint8_t m_host_players;

    const std::string& m_lobby_code;
    WallClock m_start_time;

    SimpleIntegerOption<uint16_t>& m_lobby_wait_delay;
    IntegerEnumDropdownOption& m_start_raid_players;
    EventNotificationOption& m_notification_raid_start;

    //  Sensors
    AdvanceDialogWatcher m_dialog;
    WhiteScreenOverWatcher m_start_raid;
    TeraLobbyJoinWatcher2 m_join_watcher;
    TeraLobbyNameWatcher m_name_watcher;

    //  Last known state
    uint8_t m_total_players = 1;
//    uint8_t m_ready_players = 0;
    uint8_t m_ready_joiners = 0;
    std::array<std::map<Language, std::string>, 4> m_names;
    std::vector<TeraLobbyNameMatchResult> m_bans;
    WallClock m_ban_timer = WallClock::max();
};




}
}
}
#endif
