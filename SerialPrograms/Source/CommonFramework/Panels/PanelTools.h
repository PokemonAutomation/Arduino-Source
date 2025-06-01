/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Panel_H
#define PokemonAutomation_Panel_H

#include <memory>
#include "PanelDescriptor.h"

namespace PokemonAutomation{

class Logger;
class JsonValue;
class PanelInstance;
class PanelDescriptor;


struct PanelHolder{
    //  Returns true if ready for new panel.
    virtual bool report_new_panel_intent(const PanelDescriptor& descriptor) = 0;

    virtual void load_panel(
        std::shared_ptr<const PanelDescriptor> descriptor,
        std::unique_ptr<PanelInstance> panel
    ) = 0;
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


// Used by `make_panel()` to link the panel instance to the panel descriptor.
// For more details, see `make_panel()` defined below.
template <typename Descriptor, typename Instance>
class PanelDescriptorWrapper : public Descriptor{
public:
    virtual std::unique_ptr<PanelInstance> make_panel() const override{
        return std::unique_ptr<PanelInstance>(new Instance(*this));
    }
};

// Called by a program panel list factory to create a panel descriptor.
// A panel descriptor holds various info (title, title color, etc.) about a program panel
// and can also create the corresponding panel.
//
// template type `Descriptor` a derived class of CommonFramework/Panels/PanelDescriptor.h:PanelDescriptor
// and `Instance` is the program panel UI instance, derived class of 
// CommonFramework/Panels/PanelInstance.h:Panelnstance.
//
// Each panel descriptor instance should have implemented `make_panel()` to create the panel instance.
// But writing this creation for each unique program implementation is repetitive. In stead, this function
// uses `PanelDescriptorWrapper` to implement `make_panel()`.
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
