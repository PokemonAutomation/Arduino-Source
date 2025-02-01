/*  Null Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_NullController_H
#define PokemonAutomation_Controllers_NullController_H

#include "ControllerDescriptor.h"

namespace PokemonAutomation{



class NullControllerDescriptor : public ControllerDescriptor{
public:
    static const char TYPENAME[];

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



}
#endif
