/*  Join Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_JoinTracker_H
#define PokemonAutomation_PokemonSV_JoinTracker_H

#include <string>
#include <array>
#include <set>
#include <map>
#include <mutex>
#include "CommonFramework/Language.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  On the lobby of a Tera raid, check if any of the players are banned.
//  Returns the # of players banned.
uint8_t check_ban_list(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& match_list,
    const std::vector<PlayerListRowSnapshot>& ban_list,
    const std::array<std::map<Language, std::string>, 4>& player_names,
    bool include_host, bool ignore_whitelist
);




class JoinReportRow : public StaticTableRow{
public:
    JoinReportRow(Language p_language, bool p_enabled);

    LabelCellOption language;
    BooleanCheckBoxCell enabled;
};
class JoinReportTable : public StaticTableOption{
public:
    JoinReportTable();
    virtual std::vector<std::string> make_header() const;
};



class RaidJoinReportOption : public GroupOption{
public:
    RaidJoinReportOption();

public:
    StaticTextOption text;
    JoinReportTable table;
    BooleanCheckBoxOption wins_only;
};




class JoinTracker{
public:
    void add(const std::string& name, std::string lobby_code);
    std::string dump(size_t indent = 0) const;

private:
    std::map<std::string, std::set<std::string>> m_database;
};

class MultiLanguageJoinTracker{
public:
    MultiLanguageJoinTracker(uint8_t host_players);

    void add(Language language, const std::string& name, std::string lobby_code);
    void append(
        const std::array<std::map<Language, std::string>, 4>& names,
        const std::string& lobby_code
    );

    std::string dump() const;
    void dump(const std::string& filename) const;

private:
    uint8_t m_host_players;
    std::map<Language, JoinTracker> m_databases;
};




class TeraLobbyJoinWatcher2 : public TeraLobbyReader, public VisualInferenceCallback{
public:
    TeraLobbyJoinWatcher2(
        Logger& logger, Color color,
        uint8_t host_players
    );

    uint8_t last_known_total_players() const{ return m_last_known_total_players.load(std::memory_order_relaxed); }
//    uint8_t last_known_ready_players() const{ return m_last_known_ready_players.load(std::memory_order_relaxed); }
    uint8_t last_known_ready_joiners() const{ return m_last_known_ready_joiners.load(std::memory_order_relaxed); }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    uint8_t m_host_players;
    std::atomic<uint8_t> m_last_known_total_players = 0;
//    std::atomic<uint8_t> m_last_known_ready_players = 0;
    std::atomic<uint8_t> m_last_known_ready_joiners = 0;
};


class TeraLobbyNameWatcher : public TeraLobbyReader, public VisualInferenceCallback{
public:
    TeraLobbyNameWatcher(
        Logger& logger, Color color,
        RaidJoinReportOption& report_settings,
        RaidPlayerBanList& ban_settings,
        uint8_t host_players
    );

    void get_last_known_state(
        std::array<std::map<Language, std::string>, 4>& names,
        std::vector<TeraLobbyNameMatchResult>& bans
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    RaidJoinReportOption& m_report_settings;
    RaidPlayerBanList& m_ban_settings;
    uint8_t m_host_players;

    mutable std::mutex m_lock;
    std::array<std::map<Language, std::string>, 4> m_last_known_names;
    std::vector<TeraLobbyNameMatchResult> m_last_known_bans;
};






}
}
}
#endif
