/*  Options Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OptionsPanel_H
#define PokemonAutomation_OptionsPanel_H

#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/BatchOption.h"
#include "CommonFramework/Panels/PanelSession.h"

namespace PokemonAutomation{


class OptionsPanelInstance;



class OptionsPanelDescriptor : public PanelDescriptor{
public:
    OptionsPanelDescriptor(
        Color color,
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        PanelDeprecation deprecation = PanelDeprecation::NOT_DEPRECATED,
        bool restore_defaults_button = true
    );

    bool restore_defaults_button() const{ return m_restore_defaults_button; }

    virtual std::unique_ptr<PanelSession> make_panel() const override;
    virtual std::unique_ptr<OptionsPanelInstance> make_instance() const = 0;


private:
    const bool m_restore_defaults_button;
};



class OptionsPanelInstance{
public:
    virtual ~OptionsPanelInstance() = default;
    OptionsPanelInstance(const OptionsPanelInstance&) = delete;
    void operator=(const OptionsPanelInstance&) = delete;


public:
    OptionsPanelInstance();


public:
    //  Serialization
    virtual void restore_defaults();
    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);


protected:
    friend class OptionsPanelSession;

    BatchOption m_options;
    void add_option(ConfigOption& option, std::string serialization_string);
};




template <typename Descriptor, typename Instance>
class OptionsPanelWrapper : public Descriptor{
public:
    virtual std::unique_ptr<OptionsPanelInstance> make_instance() const override{
        return std::make_unique<Instance>();
    }
};


template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_OptionsPanel(){
    return std::make_unique<OptionsPanelWrapper<Descriptor, Instance>>();
}




}
#endif


