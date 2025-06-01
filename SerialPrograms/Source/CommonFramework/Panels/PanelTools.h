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

// Abstract base class of a panel holder.
// It is named as the owner of all the panel instances.
// A panel instance, CommonFramework/Panels/PanelInstance.h:PanelInstance holds
// both the program panel UI and the implementation of the actual program logic.
//
// Currently the main window is the only class that implements PanelHolder.
// The reference of this panel holder is passed to various UI objects so that
// when they do sth they can call back to the main window. e.g. when the progra
// start button is pressed by user, the button code needs to lock the program
// list UI. This is achieved by letting the button code calls PanelHolder::on_busy()
// which is implemented by the main window
struct PanelHolder{
    //  Returns true if ready for new panel.
    virtual bool report_new_panel_intent(const PanelDescriptor& descriptor) = 0;

    virtual void load_panel(
        std::shared_ptr<const PanelDescriptor> descriptor,
        std::unique_ptr<PanelInstance> panel
    ) = 0;
    virtual Logger& raw_logger() = 0;
    // called when an automation program is running
    virtual void on_busy() = 0;
    // called when no automation program is not running
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
    // Instance must be an inherited class of PanelInstance and its constructor must be
    // Instance(const Descriptor&)
    virtual std::unique_ptr<PanelInstance> make_panel() const override{
        return std::make_unique<Instance>(*this);
    }
};

// Called by a program panel list factory to create a panel descriptor.
// A panel descriptor holds various info (title, title color, etc.) about a program panel
// and can also create the corresponding panel.
//
// template type `Descriptor` is a derived class of CommonFramework/Panels/PanelDescriptor.h:PanelDescriptor
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
