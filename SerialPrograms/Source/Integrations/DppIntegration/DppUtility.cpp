#ifdef PA_DPP

#include <dpp/DPP_SilenceWarnings.h>
#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppUtility.h>
#include "CommonFramework/GlobalSettingsPanel.h"

using namespace dpp;
namespace PokemonAutomation{
namespace Integration{
namespace DppUtility{

std::map<std::string, std::atomic_int> Utility::user_counts;

Logger& Utility::dpp_logger() {
    static TaggedLogger logger(global_logger_raw(), "DPP");
    return logger;
}

void Utility::log(const std::string& message, const std::string& identity, const loglevel& ll) {
    std::string log = identity + (std::string)": " + message;
    Color color;
    switch (ll) {
        case ll_debug: color = COLOR_CYAN; break;
        case ll_error: color = COLOR_RED; break;
        case ll_critical: color = COLOR_MAGENTA; break;
        default: color = COLOR_PURPLE; break;
    };

    dpp_logger().log(log, color);
}

void Utility::get_user_counts(cluster& bot, const guild_create_t& event) {
    // Retrieve ID and exit early if we have already pulled members for this guild.
    auto id = std::to_string(event.created->id);
    if (!user_counts.empty() && user_counts.count(id)) {
        log("Users for " + event.created->name + " already initialized.", "get_user_counts()", ll_info);
        return;
    }

    uint32_t count = event.created->member_count;
    user_counts.emplace(id, count);
    log("User count: " + std::to_string(count) + " (" + event.created->name + ")", "get_user_counts()", ll_info);
}

uint16_t Utility::get_button(const uint16_t& bt) {
    if (bt > 13) {
        uint8_t dpad = 0;
        switch (bt) {
            case 14: dpad = 0; break; // DUP
            case 15: dpad = 4; break; // DDown
            case 16: dpad = 2; break; // DLeft
            case 17: dpad = 6; break; // DRight
            default: dpad = 0; break;
        };
        return dpad;
    }
    return 1 << bt;
}



}
}
}
#endif
