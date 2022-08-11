/*  Multi-Switch Program Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramOption_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramOption_H

#include "CommonFramework/Panels/Panel.h"
#include "NintendoSwitch_MultiSwitchSystemOption.h"

namespace PokemonAutomation{
    class ConfigOption;
namespace NintendoSwitch{

class MultiSwitchProgramDescriptor;
class MultiSwitchProgramInstance2;


class MultiSwitchProgramOption final : public PanelInstance{
public:
    MultiSwitchProgramOption(const MultiSwitchProgramDescriptor& descriptor);

    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    const MultiSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }
    MultiSwitchSystemOption& system(){ return m_system; }
    MultiSwitchProgramInstance2& instance(){ return *m_instance; }
    ConfigOption& options();

    std::string check_validity() const;
    void restore_defaults();

private:
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

private:
    const MultiSwitchProgramDescriptor& m_descriptor;
    MultiSwitchSystemOption m_system;
    std::unique_ptr<MultiSwitchProgramInstance2> m_instance;
};





}
}
#endif
