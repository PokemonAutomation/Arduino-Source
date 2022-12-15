/*  Join Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_JoinTracker_H
#define PokemonAutomation_PokemonSV_JoinTracker_H

#include <string>
#include <set>
#include <map>
#include "CommonFramework/Language.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"

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
    void add(Language language, const std::string& name, std::string lobby_code);

    std::string dump() const;
    void dump(const std::string& filename) const;

private:
    std::map<Language, JoinTracker> m_databases;
};




class TeraLobbyJoinWatcher : public TeraLobbyReader, public VisualInferenceCallback{
public:
    TeraLobbyJoinWatcher(
        Logger& logger, Color color,
        RaidJoinReportOption& report_settings,
        RaidPlayerBanList& ban_settings, bool include_host
    );

    //  Returns the # of players.
    uint8_t get_last_known_state(
        std::array<std::map<Language, std::string>, 4>& names,
        std::vector<TeraLobbyNameMatchResult>& bans
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

public:
    static void append_report(
        MultiLanguageJoinTracker& report,
        const std::array<std::map<Language, std::string>, 4>& names,
        const std::string& lobby_code
    );

private:
    Logger& m_logger;
    RaidJoinReportOption& m_report_settings;
    RaidPlayerBanList& m_ban_settings;
    const bool m_include_host;

    mutable std::mutex m_lock;
    uint8_t m_last_known_players = 0;
    std::array<std::map<Language, std::string>, 4> m_last_known_names;
    std::vector<TeraLobbyNameMatchResult> m_last_known_bans;
};







}
}
}
#endif
