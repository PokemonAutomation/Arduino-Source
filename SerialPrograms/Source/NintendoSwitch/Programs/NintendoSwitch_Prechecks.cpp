/*  Prechecks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <utility>
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
    SkippablePrechecks skippable,
    const std::function<PrecheckOutcome(const Precheck&)>& run_check
){
    std::vector<PrecheckResult> results;
    for (const std::unique_ptr<const Precheck>& check : checks){
        if (check->policy() == PrecheckPolicy::SKIPPABLE && skippable == SkippablePrechecks::SKIP){
            logger.log(std::format("Precheck {}: skipped, skippable prechecks are off.", check->name()));
            results.emplace_back(PrecheckResult{check->name(), {PrecheckStatus::SKIPPED, "Skippable prechecks are off."}});
            continue;
        }
        logger.log(std::format("Precheck {}: {}", check->name(), check->needs()));
        PrecheckOutcome outcome = run_check(*check);
        if (outcome.status == PrecheckStatus::FAILED){
            throw UserSetupError(logger, std::move(outcome.message));
        }
        if (outcome.status == PrecheckStatus::PASSED && !outcome.message.empty()){
            logger.log(std::format("Precheck {}: passed. {}", check->name(), outcome.message));
        }
        if (outcome.status == PrecheckStatus::SKIPPED){
            logger.log(std::format("Precheck {}: skipped. {}", check->name(), outcome.message), COLOR_RED);
        }
        results.emplace_back(PrecheckResult{check->name(), std::move(outcome)});
    }
    return results;
}

std::vector<PrecheckResult> run_prechecks(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const PrecheckList& checks,
    SkippablePrechecks skippable
){
    return run_prechecks(
        env.console.logger(), checks, skippable,
        [&](const Precheck& check){
            return check.run(env, context);
        }
    );
}


}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
