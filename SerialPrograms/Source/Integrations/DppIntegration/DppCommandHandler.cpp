/*  DPP Command Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_DPP

//#include <format>
#include <unordered_set>
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/MessageAttachment.h"
#include "Integrations/IntegrationsAPI.h"
#include "Integrations/DiscordSettingsOption.h"
#include "DppUtility.h"
#include "DppCommandHandler.h"

using namespace dpp;
namespace PokemonAutomation{
namespace Integration{
namespace DppCommandHandler{

user Handler::owner;
Color Handler::color = COLOR_WHITE;


void Handler::initialize(cluster& bot, commandhandler& handler){
    global_logger_tagged().log("Initializing DPP...");

    bot.on_log([this](const log_t& log){
        log_dpp(log.message, "Internal Log", log.severity);
    });

#if DPP_VERSION_LONG >= 0x00100100 // (dpp version 10.1.0)
    // Do nothing, owner will be set below in on_ready callback
#else
    owner = bot.current_application_get_sync().owner;
#endif
    auto cmd_type = GlobalSettings::instance().DISCORD->integration.command_type.get();
    std::string prefix = GlobalSettings::instance().DISCORD->integration.command_prefix;

    if (cmd_type == DiscordIntegrationSettingsOption::CommandType::MessageCommands && !prefix.empty()){
        handler.add_prefix(prefix);
    }else{
        handler.add_prefix("/");
    }

    bot.on_ready([&bot, &handler, this](const ready_t&){
#if DPP_VERSION_LONG >= 0x00100100 // (dpp version 10.1.0)
        log_dpp("Logged in as: " + bot.me.format_username() + ".", "Ready", ll_info);
        Handler::create_unified_commands(handler);
        bot.current_application_get([&](const dpp::confirmation_callback_t& cc){
            if (cc.is_error()){
                log_dpp("Error getting application details: " + cc.get_error().message, "Current App", ll_error);
                return;
            }
            dpp::application app = cc.get<dpp::application>();
            log_dpp("Application Name: " + app.name, "Current App", ll_info);
            log_dpp("Application ID: " + std::to_string(app.id), "Current App", ll_info);
            owner = app.owner;
            bot.set_presence(
                presence(
                    presence_status::ps_online,
                    activity_type::at_game,
                    (std::string)GlobalSettings::instance().DISCORD->integration.game_status
                )
            );
        });
#else
        log_dpp("Logged in as: " + bot.current_user_get_sync().format_username() + ".", "Ready", ll_info);
        Handler::create_unified_commands(handler);
#endif
    });

    bot.on_guild_create([&bot, this](const guild_create_t& event){
        try{
#if DPP_VERSION_LONG >= 0x00100100 // (dpp version 10.1.0)
            std::string id = std::to_string(event.created.id);
            log_dpp("Loaded guild: " + event.created.name + " (" + id + ").", "Guild Create", ll_info);
#else
            std::string id = std::to_string(event.created->id);
            log_dpp("Loaded guild: " + event.created->name + " (" + id + ").", "Guild Create", ll_info);
#endif
            std::lock_guard<std::mutex> lg(m_count_lock);
            Utility::get_user_counts(bot, event);
        }catch (std::exception& e){
            log_dpp("Failed to get user counts: " + (std::string)e.what(), "Guild Create", ll_error);
        }
    });

    if (cmd_type == DiscordIntegrationSettingsOption::CommandType::MessageCommands){
        bot.on_message_create([&handler](const message_create_t& event){
            std::string content = event.msg.content;
            if (!event.msg.author.is_bot() && handler.string_has_prefix(content)){
                auto channels = GlobalSettings::instance().DISCORD->integration.channels.command_channels();
                auto channel = std::find(channels.begin(), channels.end(), std::to_string(event.msg.channel_id));
                if (channel != channels.end()){
                    handler.route(event);
                }
            }
        });
    }

    bot.on_slashcommand([&handler](const slashcommand_t& event){
        if (!event.command.usr.is_bot() && handler.slash_commands_enabled){
            auto channels = GlobalSettings::instance().DISCORD->integration.channels.command_channels();
            auto channel = std::find(channels.begin(), channels.end(), std::to_string(event.command.channel_id));
            if (channel != channels.end()){
                handler.route(event);
            }
        }
    });
}

void Handler::send_message(
    cluster& bot,
    embed& embed,
    const std::string& channel,
    std::chrono::milliseconds delay,
    const std::string& msg,
    std::shared_ptr<PendingFileSend> file
){
    Handler::m_queue.add_event(delay > std::chrono::milliseconds(10000) ? std::chrono::milliseconds(0) : delay,
    [&bot, this, embed = std::move(embed), channel = channel, msg = msg, file = std::move(file)]() mutable {
        message m;
        if (file != nullptr && !file->filepath().empty() && !file->filename().empty()){
            std::string data;
            std::string path = file->filepath();
            try{
                data = utility::read_file(path);
                m.add_file(file->filename(), data);
                if (path.find(".txt") == std::string::npos){
                    embed.set_image("attachment://" + file->filename());
                }
            }catch (dpp::exception& e){
                log_dpp("Exception thrown while reading screenshot data: " + (std::string)e.what(), "send_message()", ll_error);
            }
        }

        if (!msg.empty() && msg != ""){
            m.content = msg;
        }

        m.allowed_mentions.parse_users = true;
        m.channel_id = channel;
        m.add_embed(embed);
        bot.message_create(m);
    });
    log_dpp("Sending message...", "send_message()", ll_info);
}

void Handler::update_response(const dpp::command_source& src, dpp::embed& embed, const std::string& msg, std::shared_ptr<PendingFileSend> file){
    message m;
    if (file != nullptr && !file->filepath().empty() && !file->filename().empty()){
        std::string data;
        try{
            data = utility::read_file(file->filepath());
            m.add_file(file->filename(), data);
            embed.set_image("attachment://" + file->filename());
        }catch (dpp::exception& e){
            log_dpp("Exception thrown while reading screenshot data: " + (std::string)e.what(), "send_message()", ll_error);
        }
    }

    if (!msg.empty() && msg != ""){
        m.content = msg;
    }

    m.add_embed(embed);
    if (src.interaction_event.has_value()){
        src.interaction_event.value().edit_response(m);
    }else{
        src.message_event.value().reply(m);
    }
}

void Handler::log_dpp(const std::string& message, const std::string& identity, const dpp::loglevel& ll){
    Utility::log(message, identity, ll);
}

bool Handler::check_if_empty(const DiscordSettingsOption& settings){
    if (!settings.integration.enabled()){
        return false;
    }
    if (((std::string)settings.integration.token).empty()){
        log_dpp("\"Token\" must not be empty. Stopping...", "check_if_empty()", loglevel::ll_error);
        return false;
    }else if (((std::string)settings.integration.token).find(",") != std::string::npos){
        log_dpp("\"Token\" must only contain one token. Stopping...", "check_if_empty()", loglevel::ll_error);
        return false;
    }
    return true;
}

uint8_t Handler::get_min_parameters(const dpp::parameter_registration_t& params){
    uint8_t c = 0;
    for (const auto& item : params){
        if (item.second.optional){
            break;
        }
        c++;
    }
    return c;
}

void Handler::add_command_hi(dpp::commandhandler& handler){
    handler.add_command(
        "hi",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            message message;
            message.set_content((std::string)GlobalSettings::instance().DISCORD->integration.hello_message);
            if (src.message_event.has_value()){
                message.set_reference(src.message_event.value().msg.id);
            }
            handler.reply(message, src);
        },
        "Hi!"
    );
}
void Handler::add_command_ping(dpp::commandhandler& handler){
    handler.add_command(
        "ping",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            handler.reply(message("Pong! :ping_pong:"), src);
        },
        "Ping pong!"
    );
}
void Handler::add_command_about(dpp::commandhandler& handler){
    handler.add_command(
        "about",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            embed embed;
            embed.set_color((uint32_t)color).set_title("Here's a little bit about me!");

            int counts = 0;
            if (!Utility::user_counts.empty()){
                for (auto& count : Utility::user_counts){
                    counts += count.second;
                }
            }

            embed.add_field("Owner", owner.format_username() + "(" + std::to_string(owner.id) + ")");
            embed.add_field("Guilds", std::to_string(Utility::user_counts.size()));
            embed.add_field("Users", std::to_string(counts));
            embed.add_field("Uptime", handler.owner->uptime().to_string());
            embed.add_field(
                "Powered By",
                PROGRAM_NAME + " " + PROGRAM_VERSION + " ([GitHub](" + GITHUB_LINK_URL + ")/[Discord](" + DISCORD_LINK_URL_EMBED + "))"
            );

            message message;
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Some info about me!"
    );
}
void Handler::add_command_help(dpp::commandhandler& handler){
    handler  .add_command(
        "help",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command List");

            static const std::unordered_set<std::string> base_commands{
                "hi", "ping", "about", "status", "help"
            };
            static const std::unordered_set<std::string> button_commands{
                "click", "joystick"
            };

            std::unordered_set<std::string> allowed = base_commands;
            if (GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users){
                allowed.insert(button_commands.begin(), button_commands.end());
            }

            auto& commands = handler.commands;
            for (auto& cmd : commands){
                const std::string& cmd_name = cmd.first;
                if (src.issuer.id != owner.id && allowed.find(cmd_name) == allowed.end()){
                    continue;
                }

                auto& params = cmd.second.parameters;
                std::string signature;
                if (!params.empty()){
                    signature = cmd_name + "(";
                    for (size_t i = 0; i < params.size(); ++i){
                        signature += params[i].first;
                        if (i + 1 < params.size()){
                            signature += ", ";
                        }
                    }
                    signature += ")";
                }else{
                    signature = cmd_name;
                }

                std::string param_details;
                for (auto& param : params){
                    param_details += ("\n-" + param.first + ": " + param.second.description);
                    if (!param.second.choices.empty()){
                        std::string choices;
                        for (auto& c : param.second.choices){
                            choices += c.second + ", ";
                        }
                        if (!choices.empty()){
                            choices = choices.substr(0, choices.size() - 2);
                        }
                        param_details += " (" + choices + ")";
                    }
                }
                embed.add_field(signature, param_details);
            }
            embed_footer footer;
            footer.set_text("Commands are case-sensitive!");
            embed.set_footer(footer);
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "View the command list."
    );
}
void Handler::add_command_status(dpp::commandhandler& handler){
    handler.add_command(
        "status",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_description(Integration::status()).set_title("Program Status");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "View program status."
    );
}
void Handler::add_command_screenshot(dpp::commandhandler& handler, bool full_version){
    parameter_registration_t parameters;
    if (full_version){
        parameters.insert(parameters.end(), {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")});
    }
    parameters.insert(parameters.end(), {
        {"format", param_info(pt_string, false, "Image format.",
            {
                {"0", "png"},
                {"1", "jpg"},
            }
        )},
    });
    uint8_t min_parameters = get_min_parameters(parameters);

    handler.add_command(
        full_version ? "screenshotX" : "screenshot",
        parameters,
        [=, &handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Program Screenshot");

            if (params.size() < min_parameters){
                embed.set_description("Missing command arguments.");
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            handler.thinking(src);
            std::string name = "None";

            uint8_t c = 0;
            int64_t id = -1;
            if (full_version){
                id = Utility::sanitize_integer_input(params, c++);
            }
            std::string button_input = std::get<std::string>(params[c++].second);
            int64_t format = Utility::get_value_from_input(handler, command, min_parameters - 1, button_input, name);

            std::string path;
            if (format == 0){
                path = "screenshot_slash.png";
            }else{
                path = "screenshot_slash.jpg";
            }

            std::string response = Integration::screenshot(id, path.c_str());
            if (!response.empty()){
                embed.set_description(response);
                Handler::update_response(src, embed, "", nullptr);
                return;
            }

            std::shared_ptr<PendingFileSend> file(new PendingFileSend(path, true));
            embed_footer footer;
            footer.set_text("Console ID: " + std::to_string(id) + " (" + name + ")");

            embed.set_footer(footer);
            Handler::update_response(src, embed, "", std::move(file));
        },
        "Take and upload a screenshot from the specified console."
    );
}
void Handler::add_command_start(dpp::commandhandler& handler){
    handler.add_command(
        "start",
        {
            {"id", param_info(pt_integer, true, "Console ID. Find yours by using the \"status\" command.")}
        },
        [&handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command Response");

            int64_t id = Utility::sanitize_optional_integer_input(params, 0).value_or(-1);
            std::string response = Integration::start_program(id);
            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description("Started the program for console ID " + std::to_string(id) + ".");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Start the currently selected program."
    );
}
void Handler::add_command_stop(dpp::commandhandler& handler){
    handler.add_command(
        "stop",
        {
            {"id", param_info(pt_integer, true, "Console ID. Find yours by using the \"status\" command.")}
        },
        [&handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command Response");

            int64_t id = Utility::sanitize_optional_integer_input(params, 0).value_or(-1);
            std::string response = Integration::stop_program(id);
            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description("Stopped the program for console ID " + std::to_string(id) + ".");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Stop the currently running program."
    );
}
void Handler::add_command_resetcamera(dpp::commandhandler& handler){
    handler.add_command(
        "resetcamera",
        {
            {"id", param_info(pt_integer, true, "Console ID. Find yours by using the \"status\" command.")}
        },
        [&handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command Response");

            int64_t id = Utility::sanitize_optional_integer_input(params, 0).value_or(-1);
            std::string response = Integration::reset_camera(id);
            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description("Reset the camera for console ID " + std::to_string(id) + ".");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Reset the camera."
    );
}
void Handler::add_command_resetcontroller(dpp::commandhandler& handler){
    handler.add_command(
        "resetcontroller",
        {
            {"id", param_info(pt_integer, true, "Console ID. Find yours by using the \"status\" command.")},
            {"index", param_info(pt_integer, true, "Controller index.")},
        },
        [&handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command Response");

            int64_t id = Utility::sanitize_optional_integer_input(params, 0).value_or(-1);
            int64_t index = Utility::sanitize_optional_integer_input(params, 1).value_or(0);
            std::string response = Integration::reset_controller(id, index);
            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description("Reset the serial connection for console ID " + std::to_string(id) + ".");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Reset the serial connection."
    );
}
void Handler::add_command_click(dpp::commandhandler& handler, bool full_version){
    parameter_registration_t parameters;
    if (full_version){
        parameters.insert(parameters.end(), {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")});
        parameters.insert(parameters.end(), {"index", param_info(pt_integer, false, "Controller index.")});
    }
    parameters.insert(parameters.end(), {
        {"button", param_info(pt_string, false, "Switch console button.",
            {{"0", "Y"},
            {"1", "B"},
            {"2", "A"},
            {"3", "X"},
            {"4", "L"},
            {"5", "R"},
            {"6", "ZL"},
            {"7", "ZR"},
            {"8", "Minus"},
            {"9", "Plus"},
            {"10", "LStick"},
            {"11", "RStick"},
            {"12", "Home"},
            {"13", "Capture"},
            {"14", "DUP"},
            {"15", "DDOWN"},
            {"16", "DLEFT"},
            {"17", "DRIGHT"},}
        )},
        {"milliseconds", param_info(pt_integer, true, "How long to hold the button for, in milliseconds. (defaults to 100ms)")}
    });
    uint8_t min_parameters = get_min_parameters(parameters);

    handler.add_command(
        full_version ? "clickX" : "click",
        parameters,
        [=, &handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command Response");

            if (params.size() < min_parameters){
                embed.set_description("Missing command arguments.");
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            uint8_t c = 0;
            int64_t id = -1;
            int64_t index = 0;
            if (full_version){
                id = Utility::sanitize_integer_input(params, c++);
                index = Utility::sanitize_optional_integer_input(params, c++).value_or(0);
            }
            std::string button_input = std::get<std::string>(params[c++].second);

            std::string name = "None";
            int64_t button = Utility::get_value_from_input(handler, command, full_version ? 2 : 0, button_input, name);
            int64_t milliseconds = Utility::sanitize_optional_integer_input(params, c++).value_or(100);

            if (button < 0){
                embed.set_description("No such button found: " + button_input);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            std::string response;
            if (button > 13){
                response = Integration::press_dpad2(id, index, milliseconds, Utility::get_button(button));
            }else{
                response = Integration::press_button2(id, index, milliseconds, Utility::get_button(button));
            }

            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description("Console ID " + std::to_string(id) + " pressed button " + name + ".");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Click a button for the specified console."
    );
}
void Handler::add_command_joystick(dpp::commandhandler& handler, bool full_version, JoystickSide side){
    std::string side_str = "";
    switch (side){
    case JoystickSide::NEITHER:
        side_str = "joystick";
        break;
    case JoystickSide::LEFT:
        side_str = "Lstick";
        break;
    case JoystickSide::RIGHT:
        side_str = "Rstick";
        break;
    }
    if (full_version){
        side_str += "X";
    }

    parameter_registration_t parameters;
    if (full_version){
        parameters.insert(parameters.end(), {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")});
        parameters.insert(parameters.end(), {"index", param_info(pt_integer, false, "Controller index.")});
    }
    parameters.insert(parameters.end(), {
        {"magnitude_x", param_info(pt_integer, false, "Movement amount in the horizontal direction. \"Left\" is 0, \"right\" is 255, \"neutral\" is 127.")},
        {"magnitude_y", param_info(pt_integer, false, "Movement amount in the vertical direction. \"Down\" is 0, \"up\" is 255, \"neutral\" is 127.")},
        {"milliseconds", param_info(pt_integer, true, "How long to hold the stick for, in milliseconds. (defaults to 100ms)")},
    });
    uint8_t min_parameters = get_min_parameters(parameters);

    handler.add_command(
        side_str,
        parameters,
        [=, &handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command Response");

            if (params.size() < min_parameters){
                embed.set_description("Missing command arguments.");
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            uint8_t c = 0;
            int64_t id = -1;
            int64_t index = 0;
            if (full_version){
                id = Utility::sanitize_integer_input(params, c++);
                index = Utility::sanitize_optional_integer_input(params, c++).value_or(0);
            }

            int64_t x = Utility::sanitize_integer_input(params, c++);
            int64_t y = Utility::sanitize_integer_input(params, c++);
            int64_t milliseconds = Utility::sanitize_optional_integer_input(params, c++).value_or(100);

            std::string response = Integration::press_joystick(id, index, milliseconds, side, x, y);
            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description(
                "Console ID " + std::to_string(id) + " moved (X: " + std::to_string(x) +
                ", Y: " + std::to_string(y) + ") for " + std::to_string(milliseconds) + "ms."
            );
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Click a button for the specified console."
    );
}

void Handler::create_unified_commands(commandhandler& handler){
    add_command_ping(handler);
    add_command_about(handler);
    add_command_hi(handler);
    add_command_status(handler);
    add_command_screenshot(handler, false);
    add_command_screenshot(handler, true);
    add_command_start(handler);
    add_command_stop(handler);
    add_command_resetcamera(handler);
    add_command_resetcontroller(handler);
    add_command_click(handler, false);
    add_command_click(handler, true);
    add_command_joystick(handler, false, JoystickSide::NEITHER);
    add_command_joystick(handler, false, JoystickSide::LEFT);
    add_command_joystick(handler, false, JoystickSide::RIGHT);
    add_command_joystick(handler, true, JoystickSide::NEITHER);
    add_command_joystick(handler, true, JoystickSide::LEFT);
    add_command_joystick(handler, true, JoystickSide::RIGHT);
}

}
}
}
#endif
