/*  Join Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <fstream>
#include <QFile>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "PokemonSV_JoinTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool check_ban_for_name(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& matches,
    const PlayerListRowSnapshot& entry, const std::string& ocr_name,
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
            "Gael",
//            "Dhruv",
            "Nikki",
            "Alice",
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
        entry,
        ocr_name,
        to_utf8(normalized_ocr_name),
        log10p, distance == 0,
        entry.notes
    });

    return true;
}
uint8_t check_ban_list(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& match_list,
    const std::vector<PlayerListRowSnapshot>& ban_list,
    const std::array<std::map<Language, std::string>, 4>& player_names,
    bool include_host, bool ignore_whitelist
){
    if (ban_list.empty()){
        return {};
    }

    //  Check each name against ban list.
    uint8_t banned_count = 0;
    for (const auto& name : player_names){
        if (name.empty()){
            continue;
        }
        bool banned = false;
        for (const PlayerListRowSnapshot& entry : ban_list){
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
                entry, iter->second,
                ignore_whitelist
            );
        }
        if (banned){
            banned_count++;
        }
    }

    return banned_count;
}







JoinReportRow::JoinReportRow(Language p_language, bool p_enabled)
    : StaticTableRow(language_data(p_language).code)
    , language(LockWhileRunning::UNLOCKED, language_data(p_language).name)
    , enabled(LockWhileRunning::UNLOCKED, p_enabled)
{
    PA_ADD_STATIC(language);
    PA_ADD_OPTION(enabled);
}
JoinReportTable::JoinReportTable()
    : StaticTableOption(
        "<b>Languages to Read:</b><br>"
        "Attempt to read every player's name in the following languages. "
        "Note that all the Latin-based languages are largely the same. So only English is enabled by default.",
        LockWhileRunning::UNLOCKED, false
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
    : GroupOption("Join Reports:", LockWhileRunning::UNLOCKED, true, false)
    , text(
        "Track how many times each IGN has joined and generate a report. "
        "This can be used to help identify people who join too many times for "
        "the purpose banning.<br><br>"
        "Note that these reports should not be taken as-is. Many players share "
        "the same IGN and text recognition is unreliable. So these reports "
        "should not be used for the purpose of automatic banning."
    )
    , wins_only(
        "<b>Languages to Read:</b><br>Track wins only. Ignore losses.",
        LockWhileRunning::UNLOCKED,
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



void MultiLanguageJoinTracker::add(Language language, const std::string& name, std::string lobby_code){
    m_databases[language].add(name, lobby_code);
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





TeraLobbyJoinWatcher::TeraLobbyJoinWatcher(
    Logger& logger, Color color,
    RaidJoinReportOption& report_settings,
    RaidPlayerBanList& ban_settings, bool include_host
)
    : TeraLobbyReader(color)
    , VisualInferenceCallback("TeraLobbyJoinWatcher")
    , m_logger(logger)
    , m_report_settings(report_settings)
    , m_ban_settings(ban_settings), m_include_host(include_host)
{}

uint8_t TeraLobbyJoinWatcher::get_last_known_state(
    std::array<std::map<Language, std::string>, 4>& names,
    std::vector<TeraLobbyNameMatchResult>& bans
){
    std::lock_guard<std::mutex> lg(m_lock);
    names = m_last_known_names;
    bans = m_last_known_bans;
    return m_last_known_players;
}

void TeraLobbyJoinWatcher::make_overlays(VideoOverlaySet& items) const{
    TeraLobbyReader::make_overlays(items);
}
bool TeraLobbyJoinWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  No lobby detected.
    if (!detect(frame)){
        return false;
    }

    //  Nothing enabled.
    bool bans_enabled = m_ban_settings.enabled();
    bool report_enabled = m_report_settings.enabled();
    if (!bans_enabled && !report_enabled){
        return false;
    }

    uint8_t players = total_players(frame);

    //  Get language list.
    std::set<Language> languages;
    for (const auto& item : m_report_settings.table.table()){
        const JoinReportRow& row = static_cast<const JoinReportRow&>(*item);
        if (row.enabled){
            languages.insert(language_code_to_enum(item->slug()));
        }
    }
    std::vector<PlayerListRowSnapshot> banslist = m_ban_settings.current_banlist();
    for (const auto& item :banslist){
        languages.insert(item.language);
    }

    //  Read names.
    std::array<std::map<Language, std::string>, 4> names = read_names(
        m_logger, languages, true, frame
    );

    //  Process bans.
    std::vector<TeraLobbyNameMatchResult> match_list;
    uint8_t banned = check_ban_list(
        m_logger,
        match_list,
        m_ban_settings.current_banlist(),
        names,
        m_include_host, m_ban_settings.ignore_whitelist
    );

    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_last_known_players = players;
        m_last_known_names = std::move(names);
        m_last_known_bans = std::move(match_list);
    }

    return banned != 0;
}

void TeraLobbyJoinWatcher::append_report(
    MultiLanguageJoinTracker& report,
    const std::array<std::map<Language, std::string>, 4>& names,
    const std::string& lobby_code
){
    for (size_t c = 0; c < 4; c++){
        for (const auto& item : names[c]){
            report.add(item.first, item.second, lobby_code);
        }
    }
}







}
}
}
