/*  Console Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_ConsolePanel_H
#define PokemonAutomation_ConsoleInfra_ConsolePanel_H

#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Panels/PanelDescriptor.h"

namespace PokemonAutomation{
namespace ConsoleInfra{


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
        size_t num_controllers = 1,
        bool deprecated = false,
        std::vector<std::string> required_resources = {}
    );

    bool deprecated() const{ return m_deprecated; }
    size_t num_controllers() const{ return m_num_controllers; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<ConsolePanelInstance> make_instance(ConsoleSystemSession& system) const = 0;


private:
    const bool m_deprecated;
    const std::vector<std::string> m_required_resources;
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
    //  Settings

    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);

    virtual std::string check_validity() const;
    virtual void restore_defaults();


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

// Create a program PanelDescriptor
template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_ConsolePanel(){
    return std::make_unique<ConsolePanelWrapper<Descriptor, Instance>>();
}




}
}
#endif
