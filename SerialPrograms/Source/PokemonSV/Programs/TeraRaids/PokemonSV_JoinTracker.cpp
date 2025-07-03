/*  Join Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <QFile>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "CommonTools/OCR/OCR_TextMatcher.h"
#include "PokemonSV_JoinTracker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool check_ban_for_name(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& matches,
    const PlayerListRowSnapshot& entry, const std::string& banlist_source,
    const std::string& ocr_name,
    bool ignore_whitelist
){
    std::u32string normalized_ocr_name = OCR::normalize_utf32(ocr_name);
    std::u32string normalized_ban_entry = OCR::normalize_utf32(entry.name);

    size_t distance = OCR::levenshtein_distance(
        normalized_ocr_name,
        normalized_ban_entry
    );

    if (distance >= normalized_ban_entry.size()){
        return false;
    }

    double probability = OCR::random_match_probability(
        normalized_ban_entry.size(),
        normalized_ban_entry.size() - distance,
        language_data(entry.language).random_match_chance
    );
    double log10p = std::log10(probability);

    //  Not a match. Return now.
    if (log10p > entry.log10p && distance != 0){
        return false;
    }


    if (!ignore_whitelist){
        //  Matched. Check against the whitelist.
        static const std::vector<std::string> WHITELIST{
            "Kuroneko",
            "Kurbo",
            "Gael",
            "Dhruv",
            "Nikki",
            "Dani",
            "denvoros",
            "Halazea",
        };
        for (const std::string& name : WHITELIST){
            std::u32string normalized_white_entry = OCR::normalize_utf32(name);
            size_t w_distance = OCR::levenshtein_distance(
                OCR::normalize_utf32(name),
                normalized_ban_entry
            );
            if (w_distance >= normalized_white_entry.size()){
                continue;
            }
            double w_probability = OCR::random_match_probability(
                normalized_white_entry.size(),
                normalized_white_entry.size() - w_distance,
                language_data(entry.language).random_match_chance
            );
            double w_log10p = std::log10(w_probability);
            if (w_log10p <= entry.log10p){
                if (PreloadSettings::instance().DEVELOPER_MODE){
                    logger.log("Cannot ban whitelisted user: " + name + " (log10p = " + tostr_default(w_log10p) + ")", COLOR_RED);
                }
                return false;
            }
        }
    }

    matches.emplace_back(TeraLobbyNameMatchResult{
        entry, banlist_source,
        ocr_name,
        to_utf8(normalized_ocr_name),
        log10p, distance == 0,
        entry.notes
    });

    return true;
}

bool check_name_against_banlist(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& match_list,
    const std::vector<PlayerListRowSnapshot>& banlist, const std::string& banlist_source,
    const std::map<Language, std::string>& name,
    bool ignore_whitelist
){
    bool banned = false;
    for (const PlayerListRowSnapshot& entry : banlist){
        if (!entry.enabled){
            continue;
        }
        auto iter = name.find(entry.language);
        if (iter == name.end()){
            throw InternalProgramError(
                &logger,
                PA_CURRENT_FUNCTION,
                "Name was not OCR'ed in the required language."
            );
        }
        banned |= check_ban_for_name(
            logger,
            match_list,
            entry, banlist_source,
            iter->second,
            ignore_whitelist
        );
    }
    return banned;
}
uint8_t check_ban_list(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& match_list,
    const std::vector<PlayerListRowSnapshot>& banlist_local,
    const std::vector<PlayerListRowSnapshot>& banlist_global,
    uint8_t host_players,
    const std::array<std::map<Language, std::string>, 4>& player_names,
    bool ignore_whitelist
){
    //  Check each name against ban list.
    uint8_t banned_count = 0;
    for (size_t c = host_players; c < 4; c++){
        const auto& name = player_names[c];
        if (name.empty()){
            continue;
        }
        bool banned = false;

        banned |= check_name_against_banlist(
            logger, match_list,
            banlist_local, "Banned by Host: ",
            name, ignore_whitelist
        );
        banned |= check_name_against_banlist(
            logger, match_list,
            banlist_global, "Banned Globally: ",
            name, ignore_whitelist
        );

        if (banned){
            banned_count++;
        }
    }

    return banned_count;
}







JoinReportRow::JoinReportRow(Language p_language, bool p_enabled)
    : StaticTableRow(language_data(p_language).code)
    , language(LockMode::UNLOCK_WHILE_RUNNING, language_data(p_language).name)
    , enabled(LockMode::UNLOCK_WHILE_RUNNING, p_enabled)
{
    PA_ADD_STATIC(language);
    PA_ADD_OPTION(enabled);
}
JoinReportTable::JoinReportTable()
    : StaticTableOption(
        "<b>Languages to Read:</b><br>"
        "Attempt to read every player's name in the following languages. "
        "Note that all the Latin-based languages are largely the same. So only English is enabled by default.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
{
    add_row(std::make_unique<JoinReportRow>(Language::English, true));
    add_row(std::make_unique<JoinReportRow>(Language::Japanese, true));
    add_row(std::make_unique<JoinReportRow>(Language::Spanish, false));
    add_row(std::make_unique<JoinReportRow>(Language::French, false));
    add_row(std::make_unique<JoinReportRow>(Language::German, false));
    add_row(std::make_unique<JoinReportRow>(Language::Italian, false));
    add_row(std::make_unique<JoinReportRow>(Language::Korean, true));
    add_row(std::make_unique<JoinReportRow>(Language::ChineseSimplified, true));
    add_row(std::make_unique<JoinReportRow>(Language::ChineseTraditional, true));
    finish_construction();
}
std::vector<std::string> JoinReportTable::make_header() const{
    std::vector<std::string> ret{
        "Language",
        "Enabled",
    };
    return ret;
}


RaidJoinReportOption::RaidJoinReportOption()
    : GroupOption(
        "Join Reports:",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED
    )
    , text(
        "Track how many times each IGN has joined and generate a report. "
        "This can be used to help identify people who join too many times for "
        "the purpose banning.<br><br>"
        "Note that these reports should not be taken as-is. Many players share "
        "the same IGN and text recognition is unreliable. So these reports "
        "should not be used for the purpose of automatic banning."
    )
    , wins_only(
        "<b>Track Wins Only:</b><br>Track wins only. Ignore losses and incomplete raids.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
{
    PA_ADD_STATIC(text);
    PA_ADD_OPTION(table);
    PA_ADD_OPTION(wins_only);
}







void JoinTracker::add(const std::string& name, std::string lobby_code){
    m_database[name].insert(std::move(lobby_code));
}
std::string JoinTracker::dump(size_t indent) const{
    std::multimap<size_t, const std::map<std::string, std::set<std::string>>::value_type*, std::greater<size_t>> sorted;

    for (const auto& item : m_database){
        sorted.emplace(item.second.size(), &item);
    }

    std::string ret;

    for (const auto& item : sorted){
        ret += std::string(indent, ' ');
        ret += std::to_string(item.first) + " : ";
        ret += item.second->first + " => (";
        bool first = true;
        for (const std::string& code : item.second->second){
            if (!first){
                ret += ",";
            }
            first = false;
            ret += code;
        }
        ret += ")\r\n";
    }

    return ret;
}




MultiLanguageJoinTracker::MultiLanguageJoinTracker(uint8_t host_players)
    : m_host_players(host_players)
{}


void MultiLanguageJoinTracker::add(Language language, const std::string& name, std::string lobby_code){
    m_databases[language].add(name, lobby_code);
}
void MultiLanguageJoinTracker::append(
    const std::array<std::map<Language, std::string>, 4>& names,
    const std::string& lobby_code
){
    for (size_t c = m_host_players; c < 4; c++){
        for (const auto& item : names[c]){
            add(item.first, item.second, lobby_code);
        }
    }
}
std::string MultiLanguageJoinTracker::dump() const{
    std::string ret;
    for (const auto& database : m_databases){
        ret += language_data(database.first).code + ":\r\n";
        ret += database.second.dump(4);
        ret += "\r\n";
    }
    return ret;
}
void MultiLanguageJoinTracker::dump(const std::string& filename) const{
    std::string str = dump();
    QFile file(QString::fromStdString(filename));
    file.open(QIODevice::WriteOnly);
    std::string bom = "\xef\xbb\xbf";
    file.write(bom.c_str(), bom.size());
    file.write(str.c_str(), str.size());
}






TeraLobbyJoinWatcher2::TeraLobbyJoinWatcher2(
    Logger& logger, Color color,
    uint8_t host_players
)
    : TeraLobbyReader(logger, color)
    , VisualInferenceCallback("TeraLobbyJoinWatcher2")
    , m_host_players(host_players)
{}
void TeraLobbyJoinWatcher2::make_overlays(VideoOverlaySet& items) const{
    TeraLobbyReader::make_overlays(items);
}
bool TeraLobbyJoinWatcher2::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  No lobby detected.
    if (!detect(frame)){
        return false;
    }

    uint8_t total_players = this->total_players(frame);
//    uint8_t ready_players = this->ready_players(frame);
    uint8_t ready_joiners = this->ready_joiners(frame, m_host_players);
    m_last_known_total_players.store(total_players, std::memory_order_relaxed);
//    m_last_known_ready_players.store(ready_players, std::memory_order_relaxed);
    m_last_known_ready_joiners.store(ready_joiners, std::memory_order_relaxed);
    return false;
}



TeraLobbyNameWatcher::TeraLobbyNameWatcher(
    Logger& logger,
    Color color,
    RaidJoinReportOption& report_settings,
    RaidPlayerBanList& ban_settings,
    uint8_t host_players
)
    : TeraLobbyReader(logger, color)
    , VisualInferenceCallback("TeraLobbyNameWatcher")
    , m_logger(logger)
    , m_report_settings(report_settings)
    , m_ban_settings(ban_settings)
    , m_host_players(host_players)
{}

void TeraLobbyNameWatcher::get_last_known_state(
    std::array<std::map<Language, std::string>, 4>& names,
    std::vector<TeraLobbyNameMatchResult>& bans
){
    std::lock_guard<std::mutex> lg(m_lock);
    names = m_last_known_names;
    bans = m_last_known_bans;
}
void TeraLobbyNameWatcher::make_overlays(VideoOverlaySet& items) const{
    TeraLobbyReader::make_overlays(items);
}
bool TeraLobbyNameWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  No lobby detected.
    if (!detect(frame)){
        return false;
    }

    bool bans_enabled = m_ban_settings.enabled();
    bool report_enabled = m_report_settings.enabled();

    //  Get language list.
    std::set<Language> languages;
    languages.insert(Language::English);
    if (report_enabled){
        for (const auto& item : m_report_settings.table.table()){
            const JoinReportRow& row = static_cast<const JoinReportRow&>(*item);
            if (row.enabled){
                languages.insert(language_code_to_enum(item->slug()));
            }
        }
    }

    std::vector<PlayerListRowSnapshot> banlist_local = m_ban_settings.banlist_local();
    std::vector<PlayerListRowSnapshot> banlist_global = m_ban_settings.banlist_global();
//    std::vector<PlayerListRowSnapshot> banslist = m_ban_settings.banlist_combined();
    if (bans_enabled){
        for (const auto& item : banlist_local){
            languages.insert(item.language);
        }
        for (const auto& item : banlist_global){
            languages.insert(item.language);
        }
    }

    //  Read names.
    std::array<std::map<Language, std::string>, 4> names = read_names(
        m_logger, languages, frame
    );

    //  Process bans.
    std::vector<TeraLobbyNameMatchResult> match_list;
    if (bans_enabled){
        check_ban_list(
            m_logger,
            match_list,
            banlist_local,
            banlist_global,
            m_host_players,
            names,
            m_ban_settings.ignore_whitelist
        );
    }

    {
        std::lock_guard<std::mutex> lg(m_lock);
//        m_last_known_players.store(players, std::memory_order_relaxed);
        m_last_known_names = std::move(names);
        m_last_known_bans = std::move(match_list);
    }

    return false;
}










}
}
}
