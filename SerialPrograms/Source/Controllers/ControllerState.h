/*  Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerState_H
#define PokemonAutomation_Controllers_ControllerState_H

#include "Common/Cpp/Time.h"

namespace PokemonAutomation{

class JsonValue;
class CancellableScope;
class AbstractController;






class ControllerState{
public:
    virtual ~ControllerState() = default;

    virtual void clear() = 0;

    virtual bool operator==(const ControllerState& x) const = 0;
    virtual bool operator!=(const ControllerState& x) const{ return !(*this == x); }

    virtual bool is_neutral() const = 0;

public:
    //  Serialization
    virtual void load_json(const JsonValue& json);
    virtual JsonValue to_json() const;

public:
    //  Execution
    virtual void execute(
        CancellableScope& scope,
        AbstractController& controller,
        Milliseconds duration
    ) const;
    virtual std::string to_cpp(Milliseconds hold, Milliseconds release) const;
};








}
#endif
