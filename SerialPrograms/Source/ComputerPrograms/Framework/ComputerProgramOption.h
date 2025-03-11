/*  Computer Program Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a computer program.
 *  This class maintains no UI and is not thread-safe.
 *
 *  Note that this class does own the "ComputerProgramInstance", object
 *  which is controlled by the individual program itself. There the running
 *  program can do whatever it wants - including keeping run-time state.
 *
 */

#ifndef PokemonAutomation_ComputerPrograms_ComputerProgramOption_H
#define PokemonAutomation_ComputerPrograms_ComputerProgramOption_H

#include "CommonFramework/Panels/PanelInstance.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{

class ConfigOption;
class ComputerProgramDescriptor;
class ComputerProgramInstance;



class ComputerProgramOption final : public PanelInstance{
public:
    ComputerProgramOption(const ComputerProgramDescriptor& descriptor);

    virtual void from_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    const ComputerProgramDescriptor& descriptor() const{ return m_descriptor; }
    ComputerProgramInstance& instance(){ return *m_instance; }
    ConfigOption& options();

    std::string check_validity() const;
    void restore_defaults();

private:
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;

private:
    const ComputerProgramDescriptor& m_descriptor;
    std::unique_ptr<ComputerProgramInstance> m_instance;
};




}
#endif
