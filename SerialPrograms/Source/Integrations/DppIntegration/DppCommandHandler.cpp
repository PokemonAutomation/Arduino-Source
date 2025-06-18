#ifdef PA_DPP

#include <format>
#include <dpp/dpp.h>
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
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

    owner = bot.current_application_get_sync().owner;
    auto cmd_type = GlobalSettings::instance().DISCORD->integration.command_type.get();
    std::string prefix = GlobalSettings::instance().DISCORD->integration.command_prefix;

    if (cmd_type == DiscordIntegrationSettingsOption::CommandType::MessageCommands && !prefix.empty()){
        handler.add_prefix(prefix);
    }else{
        handler.add_prefix("/").add_prefix("_cmd ");
    }

    bot.on_ready([&bot, &handler, this](const ready_t&){
        log_dpp("Logged in as: " + bot.current_user_get_sync().format_username() + ".", "Ready", ll_info);
        Handler::create_unified_commands(handler);
    });

    bot.on_guild_create([&bot, this](const guild_create_t& event){
        try{
            std::string id = std::to_string(event.created->id);
            log_dpp("Loaded guild: " + event.created->name + " (" + id + ").", "Guild Create", ll_info);
            std::lock_guard<std::mutex> lg(m_count_lock);
            Utility::get_user_counts(bot, event);
        }catch (std::exception& e){
            log_dpp("Failed to get user counts: " + (std::string)e.what(), "Guild Create", ll_error);
        }
    });

    bot.on_guild_member_add([this](const guild_member_add_t& event){
        std::string id = std::to_string(event.adding_guild->id);
        if (!user_counts.empty() && user_counts.count(id)){
            log_dpp("New member joined " + event.adding_guild->name + ". Incrementing member count.", "Guild Member Add", ll_info);
            user_counts.at(id)++;
        }
    });

    bot.on_guild_member_remove([this](const guild_member_remove_t& event){
        std::string id = std::to_string(event.removing_guild->id);
        if (!user_counts.empty() && user_counts.count(id)){
            log_dpp("Member left " + event.removing_guild->name + ". Decrementing member count.", "Guild Member Remove", ll_info);
            user_counts.at(id)--;
        }
    });

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

void Handler::send_message(cluster& bot, embed& embed, const std::string& channel, std::chrono::milliseconds delay, const std::string& msg, std::shared_ptr<PendingFileSend> file){
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
            }catch (dpp::exception e){
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
        }catch (dpp::exception e){
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

void Handler::create_unified_commands(commandhandler& handler){
    handler
    .add_command(
        "ping",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            handler.reply(message("Pong! :ping_pong:"), src);
        },
        "Ping pong!")

    .add_command(
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
        "Some info about me!")

    .add_command(
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
        "Hi!")

    .add_command(
        "resetserial",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")}
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

            int64_t id = Utility::sanitize_integer_input(params, 0);
            std::string response = Integration::reset_serial(id);
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
        "Reset the serial connection.")

    .add_command(
        "resetcamera",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")}
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

            int64_t id = Utility::sanitize_integer_input(params, 0);
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
        "Reset the camera.")

    .add_command(
        "start",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")}
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

            int64_t id = Utility::sanitize_integer_input(params, 0);
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
        "Start the currently selected program.")

    .add_command(
        "stop",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")}
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

            int64_t id = Utility::sanitize_integer_input(params, 0);
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
        "Stop the currently running program.")

    .add_command(
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
        "View program status.")

    .add_command(
        "click",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")},
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
            {"ticks", param_info(pt_integer, false, "How long to hold the button for, in ticks.")},
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

            if (params.size() < 3){
                embed.set_description("Missing command arguments.");
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            int64_t id = Utility::sanitize_integer_input(params, 0);
            std::string button_input = std::get<std::string>(params[1].second);

            std::string name = "None";
            int64_t button = Utility::get_value_from_input(handler, command, button_input, name);
            int64_t ticks = Utility::sanitize_integer_input(params, 2);

            if (button < 0){
                embed.set_description("No such button found: " + button_input);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            std::string response;
            if (button > 13){
                response = Integration::press_dpad(id, Utility::get_button(button), ticks);
            }else{
                response = Integration::press_button(id, Utility::get_button(button), ticks);
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
        "Click a button for the specified console.")
                
    .add_command(
        "joystick",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")},
            {"stick", param_info(pt_string, false, "Switch console joystick.",
                {{"0", "LStick"},
                {"1", "RStick"},}
            )},
            {"magnitude_x", param_info(pt_integer, false, "Movement amount in the horizontal direction. \"Left\" is 0, \"right\" is 255, \"neutral\" is 127.")},
            {"magnitude_y", param_info(pt_integer, false, "Movement amount in the vertical direction. \"Down\" is 0, \"up\" is 255, \"neutral\" is 127.")},
            {"ticks", param_info(pt_integer, false, "How long to hold the stick for, in ticks.")},
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

            if (params.size() < 5){
                embed.set_description("Missing command arguments.");
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            std::string name = "None";
            int64_t id = Utility::sanitize_integer_input(params, 0);
            std::string stick_input = std::get<std::string>(params[1].second);
            int64_t stick = Utility::get_value_from_input(handler, command, stick_input, name);

            if (stick < 0){
                embed.set_description("No such joystick found: " + stick_input);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            int64_t x = Utility::sanitize_integer_input(params, 2);
            int64_t y = Utility::sanitize_integer_input(params, 3);
            int64_t ticks = Utility::sanitize_integer_input(params, 4);

            std::string response;
            if (stick == 0){
                response = Integration::press_left_joystick(id, x, y, ticks);
            }else{
                response = Integration::press_right_joystick(id, x, y, ticks);
            }

            if (!response.empty()){
                embed.set_description(response);
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            embed.set_description("Console ID " + std::to_string(id) + " moved " + name + " (X: " + std::to_string(x) + ", Y: " + std::to_string(y) + ") for " + std::to_string(ticks) + " ticks.");
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "Click a button for the specified console.")
            
    .add_command(
        "screenshot",
        {
            {"id", param_info(pt_integer, false, "Console ID. Find yours by using the \"status\" command.")},
            {"format", param_info(pt_string, false, "Image format.",
                {{"0", "png"},
                {"1", "jpg"},}
            )},
        },
        [&handler, this](const std::string& command, const parameter_list_t& params, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users && src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Program Screenshot");

            if (params.size() < 2){
                embed.set_description("Missing command arguments.");
                message.add_embed(embed);
                handler.reply(message, src);
                return;
            }

            handler.thinking(src);
            std::string name = "None";
            int64_t id = Utility::sanitize_integer_input(params, 0);
            std::string button_input = std::get<std::string>(params[1].second);
            int64_t format = Utility::get_value_from_input(handler, command, button_input, name);

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
        "Take and upload a screenshot from the specified console.")
            
    .add_command(
        "help",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            message message;
            embed embed;
            embed.set_color((uint32_t)color).set_title("Command List");

            auto& commands = handler.commands;
            for (auto& cmd : commands){
                std::string name = cmd.first;
                log_dpp(name, "help command", ll_info);
                if (!GlobalSettings::instance().DISCORD->integration.allow_buttons_from_users &&
                    src.issuer.id != owner.id && name != "hi" && name != "ping" && name != "about" && name != "status" && name != "help"
                ){
                    continue;
                }

                std::string param_info;
                for (auto& param : cmd.second.parameters){
                    param_info += ("\n**- " + param.first + "** - " + param.second.description);
                    if (!param.second.choices.empty()){
                        param_info += " (";
                        for (auto& choice : param.second.choices){
                            param_info += (choice.second + ", ");
                        }
                        param_info = param_info.substr(0, param_info.size() - 2);
                        param_info += ")";
                    }
                }
                embed.add_field(name, param_info);
            }
            embed_footer footer;
            footer.set_text("Commands are case-sensitive!");
            embed.set_footer(footer);
            message.add_embed(embed);
            handler.reply(message, src);
        },
        "View the command list.")
            
    .add_command(
        "register",
        {},
        [&handler, this](const std::string& command, const parameter_list_t&, command_source src){
            log_dpp("Executing " + command + "...", "Unified Command Handler", ll_info);
            if (src.issuer.id != owner.id){
                handler.reply(message("You do not have permission to use this command."), src);
                return;
            }

            if (handler.slash_commands_enabled){
                handler.thinking(src);
                log_dpp("Registering commands.", "Command Registration", ll_info);
                handler.register_commands();

                embed embed;
                std::string desc = "Slash commands registered! Restart your Discord client or wait a few minutes for them to show up!";
                embed.set_color((uint32_t)color).set_description(desc).set_title("Slash Command Registration");
                Handler::update_response(src, embed, "", nullptr);
            }else{
                handler.reply(message("Enable slash commands before registering them."), src);
            }
        },
        "Register global slash commands. For first-time slash command use and for updating commands.");
}


}
}
}
#endif
