/*  Multi-Console Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  ConsolePanel is a panel with:
 *    - Has options.
 *    - Has multiple consoles.
 *    - Cannot be run.
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsolePanel_H
#define PokemonAutomation_GameConsole_MultiConsolePanel_H

#include <functional>
#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Panels/PanelDescriptor.h"

namespace PokemonAutomation{
namespace GameConsole{

class ConsoleSystemOption;
class ConsoleSystemSession;
class MultiConsoleSystemSession;
class MultiConsolePanelInstance;



class MultiConsolePanelDescriptor : public PanelDescriptor{
public:
    using OptionFactory = std::function<
        std::unique_ptr<ConsoleSystemOption>(
            size_t console_index
        )
    >;
    using SessionFactory = std::function<
        std::unique_ptr<ConsoleSystemSession>(
            ConsoleSystemOption& option,
            size_t console_index
        )
    >;

    MultiConsolePanelDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        Color color,
        PanelDeprecation deprecation,
        size_t min_consoles,
        size_t max_consoles,
        size_t default_consoles,
        bool restore_defaults_button = true
    );
    MultiConsolePanelDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        Color color,
        PanelDeprecation deprecation,
        size_t min_consoles,
        size_t max_consoles,
        size_t default_consoles,
        bool restore_defaults_button,
        OptionFactory option_factory,
        SessionFactory session_factory
    );

    size_t min_consoles() const{ return m_min_consoles; }
    size_t max_consoles() const{ return m_max_consoles; }
    size_t default_consoles() const{ return m_default_consoles; }

    const OptionFactory& option_factory() const{ return m_option_factory; }
    const SessionFactory& session_factory() const{ return m_session_factory; }

    size_t num_controllers(size_t index) const{ return m_num_controllers[index]; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<MultiConsolePanelInstance> make_instance(MultiConsoleSystemSession& system) const = 0;


private:
    size_t m_min_consoles;
    size_t m_max_consoles;
    size_t m_default_consoles;
    std::vector<size_t> m_num_controllers;

    OptionFactory m_option_factory;
    SessionFactory m_session_factory;
};




class MultiConsolePanelInstance{
public:
    virtual ~MultiConsolePanelInstance() = default;
    MultiConsolePanelInstance(const MultiConsolePanelInstance&) = delete;
    void operator=(const MultiConsolePanelInstance&) = delete;


public:
    MultiConsolePanelInstance();


public:
    //  Serialization

    virtual std::string check_validity() const;
    virtual void restore_defaults();
    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);


protected:
    friend class MultiConsolePanelSession;

    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);

};




template <typename Instance>
class MultiConsolePanelWrapper : public Instance::Descriptor{
public:
    virtual std::unique_ptr<MultiConsolePanelInstance> make_instance(
        MultiConsoleSystemSession& system
    ) const override{
        if constexpr (std::is_constructible_v<Instance, MultiConsoleSystemSession&>){
            return std::make_unique<Instance>(system);
        }else{
            return std::make_unique<Instance>();
        }
    }
};

template <typename Instance>
std::unique_ptr<PanelDescriptor> make_MultiConsolePanel(){
    return std::make_unique<MultiConsolePanelWrapper<Instance>>();
}




}
}
#endif
