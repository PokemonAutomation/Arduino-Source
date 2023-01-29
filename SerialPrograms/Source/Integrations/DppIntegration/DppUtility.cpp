#ifdef PA_DPP

#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppUtility.h>
#include "CommonFramework/GlobalSettingsPanel.h"

using namespace dpp;
namespace PokemonAutomation {
    namespace Integration {
        namespace DppUtility {
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
                snowflake last = 0;
                uint64_t count = 0;

                // Retrieve ID and exit early if we have already pulled members for this guild.
                auto id = std::to_string(event.created->id);
                if (!user_counts.empty() && user_counts.count(id)) {
                    log("Users for " + event.created->name + " already initialized.", "get_user_counts()", ll_info);
                    return;
                }

                // Fetch the first 1000 members.
                auto map = bot.guild_get_members_sync(id, 1000, last);
                count += map.size();

                /*Loop until we get fewer than 1000 because we've reached the end at that point.
                  Determine the largest member ID for the next iteration. */
                while (map.size() == 1000) {
                    auto usr = std::max_element(map.begin(), map.end(), [](const auto& x, const auto& y) {
                        return x.second.user_id < y.second.user_id;
                    });

                    last = usr->second.user_id;
                    map = bot.guild_get_members_sync(id, 1000, last);
                    count += map.size();
                }

                user_counts.emplace(id, count);
                log("User count: " + std::to_string(count) + " (" + event.created->name + ")", "get_user_counts()", ll_info);
            }

            uint64_t Utility::sanitize_console_id_input(const slashcommand_t& event) {
                int64_t id = std::get<int64_t>(event.get_parameter("id"));
                if (id > UINT64_MAX) {
                    id = UINT64_MAX;
                }
                return id;
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
