/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SettingsPanel_H
#define PokemonAutomation_SettingsPanel_H

#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Panels/PanelSession.h"

namespace PokemonAutomation{



class SettingsPanelInstance : public UiState<SettingsPanelInstance, PanelSession>{
public:
    SettingsPanelInstance(const PanelDescriptor& descriptor);

    void add_option(ConfigOption& option, std::string serialization_string){
        m_options.add_option(option, std::move(serialization_string));
    }

public:
    //  Serialization
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

protected:
    friend class SettingsPanelWidget;
    BatchOption m_options;
};




}
#endif


