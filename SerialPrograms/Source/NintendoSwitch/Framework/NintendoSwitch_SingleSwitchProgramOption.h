/*  Single Switch Program Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramOption_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramOption_H

#include "CommonFramework/Panels/Panel.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
    class ConfigOption;
namespace NintendoSwitch{

class SingleSwitchProgramDescriptor;
class SingleSwitchProgramInstance2;


class SingleSwitchProgramOption final : public PanelInstance{
public:
    SingleSwitchProgramOption(const SingleSwitchProgramDescriptor& descriptor);

    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    const SingleSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }
    SwitchSystemOption& system(){ return m_system; }
    SingleSwitchProgramInstance2& instance(){ return *m_instance; }
    ConfigOption& options();

    std::string check_validity() const;
    void restore_defaults();

private:
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

private:
    const SingleSwitchProgramDescriptor& m_descriptor;
    SwitchSystemOption m_system;
    std::unique_ptr<SingleSwitchProgramInstance2> m_instance;
};








}
}
#endif
