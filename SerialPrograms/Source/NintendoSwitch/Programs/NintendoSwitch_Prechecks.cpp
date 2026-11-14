/*  Prechecks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch_Prechecks.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


Precheck::Precheck(std::string name, std::string needs, PrecheckPolicy policy)
    : m_name(std::move(name))
    , m_needs(std::move(needs))
    , m_policy(policy)
{}


std::vector<PrecheckResult> run_prechecks(
    Logger& logger,
    const PrecheckList& checks,
    OptionalPrechecks optional,
    const std::function<PrecheckOutcome(const Precheck&)>& run_check
){
    std::vector<PrecheckResult> results;
    for (const std::unique_ptr<const Precheck>& check : checks){
        if (check->policy() == PrecheckPolicy::OPTIONAL && optional == OptionalPrechecks::SKIP){
            logger.log("Precheck " + check->name() + ": skipped, optional prechecks are off.");
            results.emplace_back(PrecheckResult{check->name(), {PrecheckStatus::SKIPPED, "Optional prechecks are off."}});
            continue;
        }
        logger.log("Precheck " + check->name() + ": " + check->needs());
        PrecheckOutcome outcome = run_check(*check);
        if (outcome.status == PrecheckStatus::FAILED){
            throw UserSetupError(logger, std::move(outcome.message));
        }
        if (outcome.status == PrecheckStatus::SKIPPED){
            logger.log("Precheck " + check->name() + ": skipped. " + outcome.message, COLOR_RED);
        }
        results.emplace_back(PrecheckResult{check->name(), std::move(outcome)});
    }
    return results;
}

std::vector<PrecheckResult> run_prechecks(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const PrecheckList& checks,
    OptionalPrechecks optional
){
    return run_prechecks(
        env.console.logger(), checks, optional,
        [&](const Precheck& check){
            return check.run(env, context);
        }
    );
}


}
}
