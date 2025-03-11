/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DiscordWebhookSettings_H
#define PokemonAutomation_DiscordWebhookSettings_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/GroupOption.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordWebhookUrl : public EditableTableRow{
public:
    DiscordWebhookUrl(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual void load_json(const JsonValue& json) override;

public:
    BooleanCheckBoxCell enabled;
    StringCell label;
    BooleanCheckBoxCell ping;
    StringCell tags_text;
    SimpleIntegerCell<uint8_t> delay;
    StringCell url;
};

class DiscordWebhookSettingsTable : public EditableTableOption_t<DiscordWebhookUrl>{
public:
    DiscordWebhookSettingsTable();
    virtual std::vector<std::string> make_header() const override;
};





class DiscordWebhookSettingsOption : public GroupOption{
public:
    DiscordWebhookSettingsOption();

    DiscordWebhookSettingsTable urls;
    SimpleIntegerOption<uint8_t> sends_per_second;
};



}
}
#endif
