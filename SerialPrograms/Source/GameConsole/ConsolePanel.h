/*  Console Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  ConsolePanel is a panel with a console and options. It cannot be run.
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_ConsolePanel_H
#define PokemonAutomation_ConsoleInfra_ConsolePanel_H

#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Panels/PanelDescriptor.h"

namespace PokemonAutomation{
namespace GameConsole{


class ConsoleSystemSession;
class ConsolePanelInstance;



class ConsolePanelDescriptor : public PanelDescriptor{
public:
    ConsolePanelDescriptor(
        Color color,
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        bool restore_defaults_button = true,
        size_t num_controllers = 1
    );

    bool restore_defaults_button() const{ return m_restore_defaults_button; }
    size_t num_controllers() const{ return m_num_controllers; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<ConsolePanelInstance> make_instance(ConsoleSystemSession& system) const = 0;


private:
    const bool m_restore_defaults_button;
    const size_t m_num_controllers;
};




class ConsolePanelInstance{
public:
    virtual ~ConsolePanelInstance() = default;
    ConsolePanelInstance(const ConsolePanelInstance&) = delete;
    void operator=(const ConsolePanelInstance&) = delete;


public:
    ConsolePanelInstance();


public:
    //  Serialization

    virtual std::string check_validity() const;
    virtual void restore_defaults();
    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);


protected:
    friend class ConsolePanelSession;

    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);

};




template <typename Descriptor, typename Instance>
class ConsolePanelWrapper : public Descriptor{
public:
    virtual std::unique_ptr<ConsolePanelInstance> make_instance(
        ConsoleSystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, ConsoleSystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_ConsolePanel(){
    return std::make_unique<ConsolePanelWrapper<Descriptor, Instance>>();
}




}
}
#endif
