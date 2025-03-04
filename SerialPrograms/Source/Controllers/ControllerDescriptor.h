/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerDescriptor_H
#define PokemonAutomation_Controllers_ControllerDescriptor_H

#include <string>
#include <memory>
#include <map>
#include "Common/Cpp/AbstractLogger.h"
//#include "Common/Cpp/Json/JsonObject.h"
#include "ControllerCapability.h"
#include "Controller.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class InterfaceType;
class ControllerDescriptor;
class ControllerConnection;
class ControllerSelectorWidget;


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
    virtual bool operator==(const ControllerDescriptor& x) const = 0;

    virtual std::string display_name() const = 0;

    virtual void load_json(const JsonValue& json) = 0;
    virtual JsonValue to_json() const = 0;

    virtual std::unique_ptr<ControllerConnection> open_connection(Logger& logger) const = 0;
    virtual std::unique_ptr<AbstractController> make_controller(
        Logger& logger,
        ControllerConnection& connection,
        ControllerType controller_type,
        const ControllerRequirements& requirements
    ) const{
        return nullptr;
    }

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


    std::shared_ptr<const ControllerDescriptor> get_descriptor_from_cache(ControllerInterface interface_type) const;


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
//    friend class ControllerSession;
    ControllerType m_controller_type;
private:
    std::shared_ptr<const ControllerDescriptor> m_descriptor;

    std::map<ControllerInterface, std::shared_ptr<const ControllerDescriptor>> m_descriptor_cache;
};








}
#endif
