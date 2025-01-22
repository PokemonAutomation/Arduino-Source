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
class ControllerConnection;



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

class NullControllerDescriptor : public ControllerDescriptor{
public:
    virtual bool operator==(const ControllerDescriptor& x) const override;
    virtual const char* type_name() const override;
    virtual std::string display_name() const override;
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<ControllerConnection> open(
        Logger& logger,
        const ControllerRequirements& requirements
    ) const override;
};




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
