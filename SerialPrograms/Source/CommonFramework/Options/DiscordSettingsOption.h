/*  Discord Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordSettings_H
#define PokemonAutomation_DiscordSettings_H

#include "BatchOption.h"
#include "BooleanCheckBoxOption.h"
#include "StringOption.h"
#include "TextEditOption.h"

namespace PokemonAutomation{


class DiscordSettingsOption : public GroupOption{
public:
    DiscordSettingsOption();
    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

    StringOption instance_name;
    BooleanCheckBoxOption ping_once;
    TextEditOption webhook_urls;
    StringOption user_id;
    StringOption message;
};

class DiscordSettingsOptionUI : public GroupOptionUI{
public:
    DiscordSettingsOptionUI(QWidget& parent, DiscordSettingsOption& value);
};

inline ConfigOptionUI* DiscordSettingsOption::make_ui(QWidget& parent){
    return new DiscordSettingsOptionUI(parent, *this);
}




}
#endif
