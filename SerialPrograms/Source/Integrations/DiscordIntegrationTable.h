/*  Discord Integration Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationTable_H
#define PokemonAutomation_DiscordIntegrationTable_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace Integration{



class DiscordIntegrationChannel2 : public EditableTableRow2{
public:
    DiscordIntegrationChannel2();
    virtual std::unique_ptr<EditableTableRow2> clone() const override;
    virtual void load_json(const JsonValue& json) override;

public:
    BooleanCheckBoxCell enabled;
    StringCell label;
    BooleanCheckBoxCell ping;
    StringCell tags_text;
    BooleanCheckBoxCell allow_commands;
    StringCell channel_id;
};

class DiscordIntegrationTable2 : public EditableTableOption2<DiscordIntegrationChannel2>{
public:
    DiscordIntegrationTable2();
    virtual std::vector<std::string> make_header() const override;
    std::vector<std::string> command_channels() const;
};






}
}
#endif
