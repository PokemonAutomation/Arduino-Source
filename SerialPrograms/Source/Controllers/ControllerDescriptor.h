/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_Controller_H
#define PokemonAutomation_Controllers_Controller_H

#include <string>
#include <memory>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{

class JsonValue;
class ControllerRequirements;
class ControllerType;
class ControllerDescriptor;
class ControllerConnection;


//
//  Represents an entire type of controller.
//
//  For example:
//      -   "Nintendo Switch via Serial PABotBase" is a type of controller.
//      -   "Nintendo Switch via Joycon Emulatiion" is a type of controller. (hypothetical)
//      -   "Xbox One via whatever..." ...
//
//  One instance of this class exists for each type of controller.
//
class ControllerType{
public:
    virtual ~ControllerType() = default;

    //  Returns a list of all available instances for this controller type.
    virtual std::vector<std::shared_ptr<const ControllerDescriptor>> list() const = 0;

    //  Construct a descriptor from a JSON config. (reloading saved controller settings)
    virtual std::unique_ptr<ControllerDescriptor> make(const JsonValue& json) const = 0;

protected:
    static void register_factory(
        const std::string& name,
        std::unique_ptr<ControllerType> factory
    );
};


//
//  Subclass helper for ControllerType.
//
template <typename DescriptorType>
class ControllerType_t : public ControllerType{
public:
    //  Subclasses must implement this function.
    virtual std::vector<std::shared_ptr<const ControllerDescriptor>> list() const override;

    //  This function is provided for you.
    virtual std::unique_ptr<ControllerDescriptor> make(const JsonValue& json) const override{
        std::unique_ptr<DescriptorType> ptr(new DescriptorType());
        ptr->load_json(json);
        return ptr;
    }

private:
    static int register_class(){
        ControllerType::register_factory(
            DescriptorType::TYPENAME,
            std::make_unique<ControllerType_t<DescriptorType>>()
        );
        return 0;
    }
    static int initializer;
};
template <typename DescriptorType>
int ControllerType_t<DescriptorType>::initializer = register_class();




//
//  Represents a user-selectable controller. When the user clicks on the
//  "Controller" dropdown, every item in that list corresponds to an instance
//  of this class.
//
class ControllerDescriptor{
public:
    virtual ~ControllerDescriptor() = default;
    virtual bool operator==(const ControllerDescriptor& x) const = 0;

    virtual const char* type_name() const = 0;
    virtual std::string display_name() const = 0;

    virtual void load_json(const JsonValue& json) = 0;
    virtual JsonValue to_json() const = 0;

    virtual std::unique_ptr<ControllerConnection> open(
        Logger& logger,
        const ControllerRequirements& requirements
    ) const = 0;
};




std::vector<std::shared_ptr<const ControllerDescriptor>>
get_compatible_descriptors(const ControllerRequirements& requirements);






//
//  A configurable option type for the descriptor.
//
class ControllerOption{
public:
    ControllerOption();

    std::shared_ptr<const ControllerDescriptor> current() const{
        return m_current;
    }

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

private:
    friend class ControllerSession;
    std::shared_ptr<const ControllerDescriptor> m_current;
};








}
#endif
