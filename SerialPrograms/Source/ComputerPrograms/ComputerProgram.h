/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_RunnableComputerProgram_H
#define PokemonAutomation_RunnableComputerProgram_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"

namespace PokemonAutomation{

class CancellableScope;
class ComputerProgramInstance;


class ComputerProgramDescriptor : public ProgramDescriptor{
public:
    ComputerProgramDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED
    );

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<ComputerProgramInstance> make_instance() const = 0;
};



class ComputerProgramInstance : public ProgramInstance{
public:
    using ProgramInstance::ProgramInstance;
    virtual void program(ProgramEnvironment& env, CancellableScope& scope) = 0;
};



template <typename Descriptor, typename Instance>
class ComputerProgramWrapper : public Descriptor{
public:
    virtual std::unique_ptr<ComputerProgramInstance> make_instance() const override{
        return std::unique_ptr<ComputerProgramInstance>(new Instance());
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_computer_program(){
    return std::make_unique<ComputerProgramWrapper<Descriptor, Instance>>();
}




}
#endif
