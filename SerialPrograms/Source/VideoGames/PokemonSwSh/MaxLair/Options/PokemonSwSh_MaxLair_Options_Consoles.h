/*  Max Lair Consoles
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_Consoles_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_Consoles_H

#include <memory>
#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Language.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "PokemonSwSh_MaxLair_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class CaughtScreenActionOption : public EnumDropdownOption<CaughtScreenAction>{
public:
    CaughtScreenActionOption(
        bool take_non_shiny, bool reset_if_high_winrate,
        std::string label, CaughtScreenAction default_action
    );
};
class CaughtScreenActionsOption : public GroupOption{
public:
    CaughtScreenActionsOption(
        bool host_tooltip, bool winrate_reset_tooltip,
        CaughtScreenAction default_no_shinies = CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE,
        CaughtScreenAction default_shiny_nonboss = CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE,
        CaughtScreenAction default_shiny_boss = CaughtScreenAction::STOP_PROGRAM
    );

    CaughtScreenActionOption no_shinies;
    CaughtScreenActionOption shiny_nonboss;
    CaughtScreenActionOption shiny_boss;
    StaticTextOption description;
};





class ConsoleSpecificOptions : public GroupOption{
public:
    ConsoleSpecificOptions(std::string label, const LanguageSet& languages, bool host);

    virtual void set_host(bool is_host);


    bool is_host;
    StaticTextOption is_host_label;
    OCR::LanguageOCROption language;
};
class ConsoleSpecificOptionsFactory{
public:
    virtual std::unique_ptr<ConsoleSpecificOptions> make(
        std::string label,
        const LanguageSet& languages,
        bool is_host
    ) const = 0;
};



class Consoles : public BatchOption, private ConfigOption::Listener{
public:
    ~Consoles();
    Consoles(const ConsoleSpecificOptionsFactory& factory);

    size_t active_consoles() const;
    void set_active_consoles(size_t consoles);

    const ConsoleSpecificOptions& operator[](size_t index) const{
        return *PLAYERS[index];
    }

    virtual void on_config_value_changed(void* object) override;

private:
    LanguageSet m_languages;
    size_t m_active_consoles;
public:
    HostingSwitch HOST;
    std::unique_ptr<ConsoleSpecificOptions> PLAYERS[4];
};




}
}
}
}
#endif
