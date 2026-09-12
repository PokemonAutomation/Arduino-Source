/*  Options Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OptionsPanelSession_H
#define PokemonAutomation_OptionsPanelSession_H

#include "CommonFramework/Panels/PanelSession.h"
#include "OptionsPanel.h"

namespace PokemonAutomation{



class OptionsPanelSession final : public UiState<OptionsPanelSession, PanelSession>{
public:
    OptionsPanelSession(const OptionsPanelDescriptor& descriptor);

    void restore_defaults();


public:
    const OptionsPanelDescriptor& descriptor() const{ return m_descriptor; }

    ConfigOption& options();


private:
    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;


private:
    const OptionsPanelDescriptor& m_descriptor;

    std::unique_ptr<OptionsPanelInstance> m_instance;
};



}
#endif
