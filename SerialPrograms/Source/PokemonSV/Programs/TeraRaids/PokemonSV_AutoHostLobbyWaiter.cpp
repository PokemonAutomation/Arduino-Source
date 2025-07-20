/*  Auto Host Lobby Waiter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "CommonTools/Async/InferenceSession.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_AutoHostLobbyWaiter.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


TeraLobbyWaiter::TeraLobbyWaiter(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    uint8_t host_players,
    const std::string& lobby_code, WallClock start_time,
    SimpleIntegerOption<uint16_t>& LOBBY_WAIT_DELAY,
    IntegerEnumDropdownOption& START_RAID_PLAYERS,
    EventNotificationOption& NOTIFICATION_RAID_START,
    RaidPlayerBanList& BAN_LIST,
    RaidJoinReportOption& JOIN_REPORT
)
    : m_env(env)
    , m_stream(stream), m_context(context)
    , m_host_players(host_players)
    , m_lobby_code(lobby_code), m_start_time(start_time)
    , m_lobby_wait_delay(LOBBY_WAIT_DELAY)
    , m_start_raid_players(START_RAID_PLAYERS)
    , m_notification_raid_start(NOTIFICATION_RAID_START)
    , m_dialog(COLOR_YELLOW)
    , m_start_raid(COLOR_BLUE)
    , m_join_watcher(stream.logger(), COLOR_RED, host_players)
    , m_name_watcher(stream.logger(), COLOR_RED, JOIN_REPORT, BAN_LIST, host_players)
{}

VideoSnapshot TeraLobbyWaiter::synchronize_state(){
    VideoSnapshot snapshot = m_stream.video().snapshot();
    m_join_watcher.process_frame(snapshot, snapshot.timestamp);
    m_name_watcher.process_frame(snapshot, snapshot.timestamp);
    m_total_players = m_join_watcher.last_known_total_players();
//    m_ready_players = m_join_watcher.last_known_ready_players();
    m_ready_joiners = m_join_watcher.last_known_ready_joiners();
    m_name_watcher.get_last_known_state(m_names, m_bans);
    return snapshot;
}


std::string TeraLobbyWaiter::check_hat_trick(){
    //  Hat trick requires only one host and a fully lobby.
    if (m_host_players != 1 || m_total_players != 4){
        return "";
    }

    //  All 3 joiners must OCR to the same name in English and be at least
    //  4 characters long. (to filter out false positives in other languages)
    auto iter1 = m_names[1].find(Language::English);
    if (iter1 == m_names[1].end()) return "";
    auto iter2 = m_names[2].find(Language::English);
    if (iter2 == m_names[2].end()) return "";
    auto iter3 = m_names[3].find(Language::English);
    if (iter3 == m_names[3].end()) return "";
    std::u32string name1 = OCR::normalize_utf32(iter1->second);
    if (name1.size() < 4) return "";
    std::u32string name2 = OCR::normalize_utf32(iter2->second);
    if (name2.size() < 4) return "";
    std::u32string name3 = OCR::normalize_utf32(iter3->second);
    if (name3.size() < 4) return "";
    if (name1 != name2 || name1 != name3){
        return "";
    }
    return iter1->second;
}
bool TeraLobbyWaiter::check_bans(bool skip_grace_period){
    if (m_bans.empty()){
        m_ban_timer = WallClock::max();
        return false;
    }
    if (skip_grace_period){
        return true;
    }

//    //  Enough people have joined.
//    if (m_total_players >= 4){
//        return true;
//    }

    //  Names are different.
    std::set<std::u32string> normalized_names;
    for (size_t c = m_host_players; c < 4; c++){
        auto iter = m_names[c].find(Language::English);
        if (iter == m_names[c].end()){
            continue;
        }
        normalized_names.insert(OCR::normalize_utf32(iter->second));
    }
    if (normalized_names.size() > 1){
        return true;
    }

    //  First detected banned user.
    if (m_ban_timer == WallClock::max()){
        m_ban_timer = current_time();
//        cout << "Start ban timer." << endl;
        return false;
    }

    //  Haven't waited long enough yet.
    if (current_time() - m_ban_timer < std::chrono::seconds(10)){
//        cout << "Ban grace period." << endl;
        return false;
    }

//    cout << "Ban passed." << endl;

    return true;
}

bool TeraLobbyWaiter::check_start_timeout(){
    uint16_t start_delay = m_lobby_wait_delay;
    if (current_time() < m_start_time + std::chrono::seconds(start_delay)){
        return false;
    }
    if (m_host_players + m_ready_joiners < m_total_players){
        return false;
    }
    return true;
}
bool TeraLobbyWaiter::check_enough_players(){
    if (m_total_players < (uint8_t)m_start_raid_players.current_value()){
        return false;
    }
    if (m_host_players + m_ready_joiners < m_total_players){
        return false;
    }
    return true;
}


bool TeraLobbyWaiter::process_hat_trick(const ImageViewRGB32& snapshot){
    std::string name = check_hat_trick();
    if (name.empty()){
        return false;
    }
    m_stream.log(name + " with the Hat Trick!", COLOR_BLUE);
    send_program_notification(
        m_env, m_notification_raid_start,
        COLOR_PURPLE,
        "\U0001FA84\U0001F3A9\u2728 " + name + " with the Hat Trick! \u2728\U0001F3A9\U0001FA84",
        {{
            "Start Reason:",
            "\U0001FA84\U0001F3A9\u2728 " + name + " Hat Trick! \u2728\U0001F3A9\U0001FA84"
        }}, "",
        snapshot
    );
    return true;
}
bool TeraLobbyWaiter::process_bans(const ImageViewRGB32& snapshot, bool skip_grace_period){
    if (!check_bans(skip_grace_period)){
        return false;
    }

    m_env.log("Detected banned user!", COLOR_RED);
    std::string message;
    for (const TeraLobbyNameMatchResult& user : m_bans){
        m_env.log("Banned User: " + user.to_str(), COLOR_RED);
        message += user.to_str();
        message += "\n";
        if (user.notes.empty()){
            message += user.banlist_source + "(no reason provided)";
        }else{
            message += user.banlist_source + user.notes;
        }
        message += "\n";
    }
    send_program_notification(
        m_env, m_notification_raid_start,
        COLOR_RED,
        m_lobby_code.empty()
            ? "Raid Cancelled Due to Banned User"
            : "Raid (" + m_lobby_code + ") Cancelled Due to Banned User",
        {{"Banned User(s):", std::move(message)}}, "",
        snapshot
    );
    pbf_press_button(m_context, BUTTON_B, 20, 230);
    pbf_press_button(m_context, BUTTON_A, 20, 230);

    return true;
}
bool TeraLobbyWaiter::process_start_timeout(const ImageViewRGB32& snapshot){
    uint16_t start_delay = m_lobby_wait_delay;
    if (current_time() < m_start_time + std::chrono::seconds(start_delay)){
        return false;
    }
    if (m_host_players + m_ready_joiners < m_total_players){
        return false;
    }

    send_program_notification(
        m_env, m_notification_raid_start,
        COLOR_GREEN,
        m_lobby_code.empty()
            ? "Tera Raid is Starting!"
            : "Tera Raid (" + m_lobby_code + ") is Starting!",
        {{
            "Start Reason:",
            "Waited more than " + std::to_string(start_delay) + " seconds."
        }}, "",
        snapshot
    );
    return true;
}
bool TeraLobbyWaiter::process_enough_players(const ImageViewRGB32& snapshot){
    uint8_t start_raid_players = (uint8_t)m_start_raid_players.current_value();
    if (m_total_players < start_raid_players){
        return false;
    }
    if (m_host_players + m_ready_joiners < m_total_players){
        return false;
    }

    m_stream.log("Enough players are ready, attempting to start raid!", COLOR_BLUE);
    send_program_notification(
        m_env, m_notification_raid_start,
        COLOR_GREEN,
        m_lobby_code.empty()
            ? "Tera Raid is Starting!"
            : "Tera Raid (" + m_lobby_code + ") is Starting!",
        {{
            "Start Reason:",
            "Lobby has reached " + std::to_string(m_total_players) + " players."
        }}, "",
        snapshot
    );
    return true;
}


TeraLobbyWaiter::LobbyResult TeraLobbyWaiter::run_lobby(){
    CancellableHolder<CancellableScope> subcontext(static_cast<CancellableScope&>(m_context));
    InferenceSession session(
        subcontext, m_stream,
        {
            m_dialog,
            m_start_raid,
            {m_join_watcher, std::chrono::seconds(1)},  //  Both of these involve OCR.
            {m_name_watcher, std::chrono::seconds(1)}   //  Let's not spam them at the full frame rate.
        }
    );

    WallClock end_time = m_start_time + std::chrono::seconds(170);
    int ret = -1;
    while (true){
        try{
            subcontext.wait_for(std::chrono::milliseconds(100));
        }catch (OperationCancelledException&){
            ret = session.triggered_index();
            break;
        }

        //  Read sensors.
        m_total_players = m_join_watcher.last_known_total_players();
//        m_ready_players = m_join_watcher.last_known_ready_players();
        m_ready_joiners = m_join_watcher.last_known_ready_joiners();
        m_name_watcher.get_last_known_state(m_names, m_bans);

//        cout << "total = " << (int)m_total_players << ", ready = " << (int)m_ready_players << endl;

        //  No events have fired. Keep looping.
        if (check_hat_trick().empty() &&
            !check_bans(false) &&
            !check_start_timeout() &&
            !check_enough_players() &&
            !(end_time < current_time())
        ){
            continue;
        }

        //  Synchronize the state.
        VideoSnapshot snapshot = synchronize_state();

        //  Now we check again with everything fully updated.

        //  Hat trick.
        if (process_hat_trick(snapshot)){
            return LobbyResult::RAID_STARTED;
        }

        //  Check bans.
        if (process_bans(snapshot, true)){
            return LobbyResult::BANNED_PLAYER;
        }

        //  Waited long enough. Start raid if everyone is ready.
        if (process_start_timeout(snapshot)){
            return LobbyResult::RAID_STARTED;
        }

        //  Enough players in and all are ready.
        if (process_enough_players(snapshot)){
            return LobbyResult::RAID_STARTED;
        }

        //  Almost out of time.
        if (end_time < snapshot.timestamp){
            m_stream.log("Clock running down, attempting to start raid!", COLOR_BLUE);
            send_program_notification(
                m_env, m_notification_raid_start,
                COLOR_GREEN,
                m_lobby_code.empty()
                    ? "Tera Raid is Starting!"
                    : "Tera Raid (" + m_lobby_code + ") is Starting!",
                {{
                    "Start Reason:",
                    "Clock is running out!"
                }}, "",
                snapshot
            );
            return LobbyResult::RAID_STARTED;
        }

        //  Otherwise, resume looping.
    }

    VideoSnapshot snapshot = m_stream.video().snapshot();
    if (ret == 0){
        m_env.log("Raid timed out!", COLOR_ORANGE);
        send_program_notification(
            m_env, m_notification_raid_start,
            COLOR_ORANGE,
            m_lobby_code.empty()
                ? "Tera Raid timed out with no joiners."
                : "Tera Raid (" + m_lobby_code + ") timed out with no joiners.",
            {}, "",
            snapshot
        );
        return LobbyResult::RAID_FAILED;
    }

    if (ret == 1){
        m_env.log("Raid unexpectedly started!", COLOR_ORANGE);
        send_program_notification(
            m_env, m_notification_raid_start,
            COLOR_ORANGE,
            m_lobby_code.empty()
                ? "Tera Raid unexpectedly started!"
                : "Tera Raid (" + m_lobby_code + ") unexpectedly started!",
            {}, "",
            snapshot
        );
        return LobbyResult::RAID_STARTED;
    }

    throw InternalProgramError(&m_stream.logger(), PA_CURRENT_FUNCTION, "Invalid session return code.");
}











}
}
}
