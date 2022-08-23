/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordWebhookSettings_H
#define PokemonAutomation_DiscordWebhookSettings_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"
#include "Common/Cpp/Options/GroupOption.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordWebhookUrl : public EditableTableRow2{
public:
    DiscordWebhookUrl();
    virtual std::unique_ptr<EditableTableRow2> clone() const override;
    virtual void load_json(const JsonValue& json) override;

public:
    BooleanCheckBoxCell enabled;
    StringCell label;
    BooleanCheckBoxCell ping;
    StringCell tags_text;
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
};



}
}
#endif
