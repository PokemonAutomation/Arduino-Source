/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Panel_H
#define PokemonAutomation_Panel_H

#include <memory>
#include "Common/Compiler.h"
#include "PanelDescriptor.h"

class QWidget;

namespace PokemonAutomation{

class Logger;
class JsonValue;
class PanelInstance;
class PanelDescriptor;


struct PanelHolder{
    //  Returns true if ready for new panel.
    virtual bool report_new_panel_intent(const PanelDescriptor& descriptor) = 0;

    virtual void load_panel(std::unique_ptr<PanelInstance> panel) = 0;
    virtual Logger& raw_logger() = 0;
    virtual void on_busy() = 0;
    virtual void on_idle() = 0;
};



struct PanelEntry{
    std::string display_name;
    std::unique_ptr<PanelDescriptor> descriptor;

    PanelEntry(std::string p_display_name)
        : display_name(std::move(p_display_name))
    {}
    PanelEntry(std::unique_ptr<PanelDescriptor> p_descriptor)
        : display_name(p_descriptor->display_name())
        , descriptor(std::move(p_descriptor))
    {}
};



template <typename Descriptor, typename Instance>
class PanelDescriptorWrapper : public Descriptor{
public:
    virtual std::unique_ptr<PanelInstance> make_panel() const override{
        return std::unique_ptr<PanelInstance>(new Instance(*this));
    }
};

template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_panel(){
    return std::make_unique<PanelDescriptorWrapper<Descriptor, Instance>>();
}
template <typename Descriptor, typename Instance>
std::unique_ptr<PanelDescriptor> make_settings(){
    auto ret = std::make_unique<PanelDescriptorWrapper<Descriptor, Instance>>();
    ret->make_panel()->from_json();
    return ret;
}




}
#endif
