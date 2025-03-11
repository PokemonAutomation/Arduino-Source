/*  Single Switch Program Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a Switch program.
 *  This class maintains no UI and is not thread-safe.
 *
 *  Note that this class does own the "SingleSwitchProgramInstance", object
 *  which is controlled by the individual program itself. There the running
 *  program can do whatever it wants - including keeping run-time state.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramOption_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramOption_H

#include "CommonFramework/Panels/PanelInstance.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
    class ConfigOption;
namespace NintendoSwitch{

class SingleSwitchProgramDescriptor;
class SingleSwitchProgramInstance;


class SingleSwitchProgramOption final : public PanelInstance{
public:
    ~SingleSwitchProgramOption();
    SingleSwitchProgramOption(const SingleSwitchProgramDescriptor& descriptor);

    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    const SingleSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }
    SwitchSystemOption& system(){ return m_system; }
    SingleSwitchProgramInstance& instance(){ return *m_instance; }
    ConfigOption& options();

    std::string check_validity() const;
    void restore_defaults();

private:
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

private:
    const SingleSwitchProgramDescriptor& m_descriptor;
    SwitchSystemOption m_system;
    std::unique_ptr<SingleSwitchProgramInstance> m_instance;
};








}
}
#endif
