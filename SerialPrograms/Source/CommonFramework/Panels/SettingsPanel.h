/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SettingsPanel_H
#define PokemonAutomation_SettingsPanel_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "Panel.h"

namespace PokemonAutomation{



class SettingsPanelInstance : public PanelInstance{
public:
    using PanelInstance::PanelInstance;

    void add_option(ConfigOption& option, std::string serialization_string){
        m_options.add_option(option, std::move(serialization_string));
    }

    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

public:
    //  Serialization
    virtual void from_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

protected:
    friend class SettingsPanelWidget;
    BatchOption m_options;
};




}
#endif


