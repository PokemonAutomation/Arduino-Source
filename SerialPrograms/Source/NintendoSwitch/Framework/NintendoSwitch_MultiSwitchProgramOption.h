/*  Multi-Switch Program Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a Switch program.
 *  This class maintains no UI and is not thread-safe.
 *
 *  Note that this class does own the "MultiSwitchProgramInstance", object
 *  which is controlled by the individual program itself. There the running
 *  program can do whatever it wants - including keeping run-time state.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramOption_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramOption_H

#include "CommonFramework/Panels/PanelInstance.h"
#include "NintendoSwitch_MultiSwitchSystemOption.h"

namespace PokemonAutomation{
    class ConfigOption;
namespace NintendoSwitch{

class MultiSwitchProgramDescriptor;
class MultiSwitchProgramInstance;


class MultiSwitchProgramOption final : public PanelInstance{
public:
    ~MultiSwitchProgramOption();
    MultiSwitchProgramOption(const MultiSwitchProgramDescriptor& descriptor);

    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    const MultiSwitchProgramDescriptor& descriptor() const{ return m_descriptor; }
    MultiSwitchSystemOption& system(){ return m_system; }
    MultiSwitchProgramInstance& instance(){ return *m_instance; }
    ConfigOption& options();

    std::string check_validity() const;
    void restore_defaults();

private:
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

private:
    const MultiSwitchProgramDescriptor& m_descriptor;
    MultiSwitchSystemOption m_system;
    std::unique_ptr<MultiSwitchProgramInstance> m_instance;
};





}
}
#endif
