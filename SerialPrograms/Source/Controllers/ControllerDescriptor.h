/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerDescriptor_H
#define PokemonAutomation_Controllers_ControllerDescriptor_H

#include <optional>
#include <string>
#include <memory>
#include <map>
#include "Common/Cpp/AbstractLogger.h"
//#include "Common/Cpp/Json/JsonObject.h"
#include "ControllerTypes.h"
#include "Controller.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class InterfaceType;
class ControllerDescriptor;
class ControllerConnection;
class ControllerSelectorWidget;



enum class ControllerResetMode{
    DO_NOT_RESET,
    SIMPLE_RESET,
    RESET_AND_CLEAR_STATE,
};



//
//  Represents an entire controller interface.
//
//  For example:
//      -   Serial PABotBase
//      -   Direct USB to X (hypothetical)
//      -   Emulator IPC (hypothetical)
//
//  One instance of this class exists for each interface.
//
class InterfaceType{
public:
    virtual ~InterfaceType() = default;

    //  Construct a descriptor from a JSON config. (reloading saved controller settings)
    virtual std::unique_ptr<ControllerDescriptor> make(const JsonValue& json) const = 0;

protected:
    static void register_factory(
        ControllerInterface controller_interface,
        std::unique_ptr<InterfaceType> factory
    );
};


//
//  Subclass helper for ControllerType.
//
template <typename DescriptorType>
class InterfaceType_t : public InterfaceType{
public:
    virtual std::unique_ptr<ControllerDescriptor> make(const JsonValue& json) const override{
        std::unique_ptr<DescriptorType> ptr(new DescriptorType());
        ptr->load_json(json);
        return ptr;
    }

private:
    static int register_class(){
        InterfaceType::register_factory(
            DescriptorType::INTERFACE_NAME,
            std::make_unique<InterfaceType_t<DescriptorType>>()
        );
        return 0;
    }
    static int initializer;
};
template <typename DescriptorType>
int InterfaceType_t<DescriptorType>::initializer = register_class();




//
//  Represents a user-selectable controller. When the user clicks on the
//  "Controller" dropdown, every item in that list corresponds to an instance
//  of this class.
//
class ControllerDescriptor{
public:
    const ControllerInterface interface_type;

    ControllerDescriptor(ControllerInterface p_interface_type)
        : interface_type(p_interface_type)
    {}
    virtual ~ControllerDescriptor() = default;

public:
    virtual bool operator==(const ControllerDescriptor& x) const = 0;
    virtual std::string display_name() const = 0;

public:
    virtual void load_json(const JsonValue& json) = 0;
    virtual JsonValue to_json() const = 0;

public:
    virtual std::unique_ptr<ControllerConnection> open_connection(
        Logger& logger,
        bool set_to_null_controller
    ) const = 0;
    virtual std::unique_ptr<AbstractController> make_controller(
        Logger& logger,
        ControllerConnection& connection,
        ControllerType controller_type,
        ControllerResetMode reset_mode
    ) const = 0;

    virtual QWidget* make_selector_QtWidget(ControllerSelectorWidget& parent) const = 0;
};








//
//  A configurable option type for the descriptor.
//
class ControllerOption{
public:
    ControllerOption();

    std::shared_ptr<const ControllerDescriptor> descriptor() const{
        return m_descriptor;
    }
    void set_descriptor(std::shared_ptr<const ControllerDescriptor> descriptor);

    //  Remember the last used descriptor for each interface type. That way when
    //  the user switches back-and-forth between two interfaces, it will reload
    //  the previous one.
    std::shared_ptr<const ControllerDescriptor> get_descriptor_from_cache(ControllerInterface interface_type) const;


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


private:
    std::shared_ptr<const ControllerDescriptor> m_descriptor;
    std::map<ControllerInterface, std::shared_ptr<const ControllerDescriptor>> m_descriptor_cache;
};








}
#endif
