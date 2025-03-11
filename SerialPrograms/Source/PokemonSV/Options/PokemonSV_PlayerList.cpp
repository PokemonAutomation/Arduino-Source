/*  Player List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV_PlayerList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


PlayerListRow::PlayerListRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , enabled(LockMode::UNLOCK_WHILE_RUNNING, true)
    , language(
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::UNLOCK_WHILE_RUNNING
    )
    , name(false, LockMode::UNLOCK_WHILE_RUNNING, "", "Ash")
    , log10p(LockMode::UNLOCK_WHILE_RUNNING, -2.5, -10, 0)
    , notes(false, LockMode::UNLOCK_WHILE_RUNNING, "", "")
{
    PA_ADD_OPTION(enabled);
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(name);
    PA_ADD_OPTION(log10p);
    PA_ADD_OPTION(notes);
}
std::unique_ptr<EditableTableRow> PlayerListRow::clone() const{
    std::unique_ptr<PlayerListRow> ret(new PlayerListRow(parent()));
    ret->enabled = enabled.current_value();
    ret->language.set(language);
    ret->name.set(name);
    ret->log10p.set(log10p);
    ret->notes.set(notes);
    return ret;
}
PlayerListRowSnapshot PlayerListRow::snapshot() const{
    PlayerListRowSnapshot entry;
    entry.enabled = enabled;
    entry.language = language;
    entry.name = name;
    entry.log10p = log10p;
    entry.notes = notes;
    return entry;
}




PlayerListTable::PlayerListTable(
    std::string label,
    LockMode lock_while_running,
    std::string notes_label
)
    : EditableTableOption_t<PlayerListRow>(
        std::move(label),
        lock_while_running,
        make_defaults()
    )
    , m_notes_label(std::move(notes_label))
{}


std::vector<std::string> PlayerListTable::make_header() const{
    return std::vector<std::string>{
        "Enabled", "Language", "Player Name", "Match Threshold (log10p)", m_notes_label
    };
}

std::vector<std::unique_ptr<EditableTableRow>> PlayerListTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
//    ret.emplace_back(std::make_unique<PlayerListRow>());
    return ret;
}
std::vector<PlayerListRowSnapshot> PlayerListTable::snapshot() const{
    return EditableTableOption_t<PlayerListRow>::snapshot<PlayerListRowSnapshot>();
}





RaidPlayerBanList::RaidPlayerBanList()
    : GroupOption(
        "Bans:",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED
    )
    , text("Ban users from this raid. If a banned person tries to join, the raid will be reset.")
    , local_table(
        "<b>Ban Table:</b><br>A table of users to ban by IGN. "
        "The last column is a tuning parameter that specifies how well the name needs to match. "
        "Text recognition is imperfect. So exact matches are rare and unreliable. "
        "The value is the estimated log10 probability of matching by chance against random characters. "
        "It is always negative. Lower value means the match needs to be more perfect to be a match.<br><br>"
        "If you are getting false positive hits, decrease this value. (make it more negative)<br>"
        "If it is failing to match, increase this value. (make it less negative)",
        LockMode::UNLOCK_WHILE_RUNNING,
        "Ban Reason (shown publicly)"
    )
    , online_table_url(
        false,
        "<b>Online Ban Table:</b><br> In addition to the above table, download a ban list from this URL.<br>"
        "Thus the list of banned players is the combination of both your local table above and the downloaded one.<br>"
        "This online ban list is automatically refreshed every raid - thus allowing the maintainer of the online list to manage bans for you.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "https://raw.githubusercontent.com/PokemonAutomation/ServerConfigs-PA-SHA/main/PokemonScarletViolet/TeraAutoHost-BanList.json",
        "https://raw.githubusercontent.com/PokemonAutomation/ServerConfigs-PA-SHA/main/PokemonScarletViolet/TeraAutoHost-BanList.json"
    )
    , ignore_whitelist(
        "<b>Ignore Whitelist:</b><br>Ignore the developer whitelist.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , online_table("", LockMode::UNLOCK_WHILE_RUNNING, "")
{
    PA_ADD_OPTION(text);
    PA_ADD_OPTION(local_table);
    PA_ADD_OPTION(online_table_url);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(ignore_whitelist);
    }
}

#if 0
std::vector<PlayerListRowSnapshot> RaidPlayerBanList::banlist_combined() const{
    std::vector<PlayerListRowSnapshot> table0 = local_table.snapshot();
    std::vector<PlayerListRowSnapshot> table1 = online_table.snapshot();
    for (PlayerListRowSnapshot& entry : table1){
        table0.emplace_back(std::move(entry));
    }
    return table0;
}
#endif
std::vector<PlayerListRowSnapshot> RaidPlayerBanList::banlist_local() const{
    return local_table.snapshot();
}
std::vector<PlayerListRowSnapshot> RaidPlayerBanList::banlist_global() const{
    return online_table.snapshot();
}

void RaidPlayerBanList::refresh_online_table(Logger& logger){
    std::string url = online_table_url;
    if (url.empty()){
        online_table.clear();
        return;
    }
    logger.log("Refreshing online ban list...");
    try{
        JsonValue json = FileDownloader::download_json_file(logger, url);
        if (json.is_null() || !json.is_array()){
            logger.log("Downloaded ban table is empty or invalid.", COLOR_RED);
            return;
        }
        size_t items = json.to_array()->size();
        logger.log("Downloaded table has " + std::to_string(items) + " row(s).");
        online_table.load_json(json);
    }catch (OperationFailedException&){}
}










}
}
}
