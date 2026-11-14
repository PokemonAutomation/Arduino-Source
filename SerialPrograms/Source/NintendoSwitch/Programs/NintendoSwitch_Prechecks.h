/*  Prechecks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  What the console and the game must look like before a program starts.
 *  A program lists its prechecks in order and runs them first thing, so a
 *  setup mistake stops the run before it resets or saves, with a message that
 *  says what to fix.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Prechecks_H
#define PokemonAutomation_NintendoSwitch_Prechecks_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


enum class PrecheckStatus{
    PASSED,
    FAILED,
    SKIPPED,
};

//  Whether a precheck can be turned off.
enum class PrecheckPolicy{
    REQUIRED,   //  Always runs.
    OPTIONAL,   //  Runs unless the program turns its optional checks off.
};

//  Whether run_prechecks() runs the OPTIONAL checks.
enum class OptionalPrechecks{
    RUN,
    SKIP,
};

//  What one precheck found. On FAILED, the message tells the user what to fix.
struct PrecheckOutcome{
    PrecheckStatus status;
    std::string message;
};

//  One precheck of a run, as run_prechecks() reports it.
struct PrecheckResult{
    std::string name;
    PrecheckOutcome outcome;
};


//  One thing a program needs before it starts.
class Precheck{
public:
    Precheck(std::string name, std::string needs, PrecheckPolicy policy);
    virtual ~Precheck() = default;

    //  Stable id, e.g. "free_party_slots".
    const std::string& name() const{ return m_name; }
    //  One line for the user: what the program needs.
    const std::string& needs() const{ return m_needs; }
    PrecheckPolicy policy() const{ return m_policy; }

    //  Reads the console. Never resets, saves or changes options.
    virtual PrecheckOutcome run(SingleSwitchProgramEnvironment& env, ProControllerContext& context) const = 0;

private:
    std::string m_name;
    std::string m_needs;
    PrecheckPolicy m_policy;
};

//  A program's prechecks, in the order they run. Building the list never
//  touches the console.
using PrecheckList = std::vector<std::unique_ptr<const Precheck>>;


//  Runs the checks in order, each through run_check, leaving out the OPTIONAL
//  ones when optional is SKIP. Throws UserSetupError with the check's message
//  at the first FAILED one. Returns one result per check.
std::vector<PrecheckResult> run_prechecks(
    Logger& logger,
    const PrecheckList& checks,
    OptionalPrechecks optional,
    const std::function<PrecheckOutcome(const Precheck&)>& run_check
);

//  Runs the checks on the console. Call first in program().
std::vector<PrecheckResult> run_prechecks(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const PrecheckList& checks,
    OptionalPrechecks optional
);


}
}
#endif
