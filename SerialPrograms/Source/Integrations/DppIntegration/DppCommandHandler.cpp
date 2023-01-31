#ifdef PA_DPP

#include <format>
#include <dpp/DPP_SilenceWarnings.h>
#include <dpp/dpp.h>
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
//#include "CommonFramework/PersistentSettings.h"
//#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Notifications/MessageAttachment.h"
#include "Integrations/IntegrationsAPI.h"
#include "Integrations/DiscordSettingsOption.h"
#include "DppUtility.h"
#include "DppCommandHandler.h"

using namespace dpp;
namespace PokemonAutomation{
namespace Integration{
namespace DppCommandHandler{


std::map<std::string, Handler::SlashCommand> Handler::command_map;
user Handler::owner;
Color Handler::color = COLOR_WHITE;

void Handler::initialize(cluster& bot) {
    bot.on_log([this](const log_t& log) {
        log_dpp(log.message, "Internal Log", log.severity);
    });

    const presence& p = presence(presence_status::ps_online, activity_type::at_game, (std::string)GlobalSettings::instance().DISCORD.integration.game_status);
    bot.set_presence(p);

    owner = bot.current_application_get_sync().owner;
    Handler::create_commands(bot);

    bot.on_ready([&bot, this](const ready_t&) {
        log_dpp("Logged in as: " + bot.current_user_get_sync().format_username() + ".", "Ready", ll_info);
        if (run_once<struct register_bot_commands>() && !m_initialized) {
            // Overwrite because we can both add and remove commands. Takes an hour for removed commands to be updated.
            log_dpp("Registering slash commands.", "Ready", ll_info);
            std::vector<slashcommand> cmds;
            for (auto& element : command_map) {
                cmds.emplace_back(element.second.command);
            }
            bot.global_bulk_command_create(cmds);
            m_initialized = true;
        }
    });

    bot.on_guild_create([&bot, this](const guild_create_t& event) {
        try {
            std::string id = std::to_string(event.created->id);
            log_dpp("Loaded guild: " + event.created->name + " (" + id + ").", "Guild Create", ll_info);
            std::lock_guard<std::mutex> lg(m_count_lock);
            Utility::get_user_counts(bot, event);
        }
        catch (std::exception& e) {
            log_dpp("Failed to get user counts: " + (std::string)e.what(), "Guild Create", ll_error);
        }
    });

    bot.on_guild_member_add([this](const guild_member_add_t& event) {
        std::string id = std::to_string(event.adding_guild->id);
        if (!user_counts.empty() && user_counts.count(id)) {
            log_dpp("New member joined " + event.adding_guild->name + ". Incrementing member count.", "Guild Member Add", ll_info);
            user_counts.at(id)++;
        }
    });

    bot.on_guild_member_remove([this](const guild_member_remove_t& event) {
        std::string id = std::to_string(event.removing_guild->id);
        if (!user_counts.empty() && user_counts.count(id)) {
            log_dpp("Member left " + event.removing_guild->name + ". Decrementing member count.", "Guild Member Remove", ll_info);
            user_counts.at(id)--;
        }
    });

    bot.on_slashcommand([this](const slashcommand_t& event) {
        auto it = command_map.find(event.command.get_command_name());
        if (it != command_map.end()) {
            log_dpp("Executing slash command: " + event.command.get_command_name() + ".", "Slash Command", ll_info);
            try {
                it->second.func(event);
            }
            catch (std::exception& e) {
                log_dpp("Exception occurred while executing command: " + event.command.get_command_name() + ".\n" + e.what(), "Slash Command", ll_critical);
            }
        }
    });
}

void Handler::send_message(cluster& bot, embed& embed, const std::string& channel, std::chrono::milliseconds delay, const std::string& msg, std::shared_ptr<PendingFileSend> file) {
    Handler::m_queue.add_event(delay > std::chrono::milliseconds(10000) ? std::chrono::milliseconds(0) : delay,
    [&bot, this, embed = std::move(embed), channel = channel, msg = msg, file = std::move(file)]() mutable {
        message m;
        if (file != nullptr && !file->filepath().empty() && !file->filename().empty()) {
            std::string data;
            try {
                data = utility::read_file(file->filepath());
                m.add_file(file->filename(), data);
                embed.set_image("attachment://" + file->filename());
            }
            catch (dpp::exception e) {
                log_dpp(std::format("Exception thrown while reading screenshot data: {}", e.what()), "send_message()", ll_error);
            }
        }

        if (!msg.empty() && msg != "") {
            m.content = msg;
        }

        m.allowed_mentions.parse_users = true;

        m.channel_id = channel;
        m.add_embed(embed);
        bot.message_create(m);
    });
    log_dpp("Sending message...", "send_message()", ll_info);
}

void Handler::update_response(const dpp::slashcommand_t& event, dpp::embed& embed, const std::string& msg, std::shared_ptr<PendingFileSend> file) {
    message m;
    if (file != nullptr && !file->filepath().empty() && !file->filename().empty()) {
        std::string data;
        try {
            data = utility::read_file(file->filepath());
            m.add_file(file->filename(), data);
            embed.set_image("attachment://" + file->filename());
        }
        catch (dpp::exception e) {
            log_dpp(std::format("Exception thrown while reading screenshot data: {}", e.what()), "send_message()", ll_error);
        }
    }

    if (!msg.empty() && msg != "") {
        m.content = msg;
    }

    m.add_embed(embed);
    event.edit_response(m);
}

void Handler::log_dpp(const std::string& message, const std::string& identity, const dpp::loglevel& ll) {
    Utility::log(message, identity, ll);
}

bool Handler::check_if_empty(const DiscordSettingsOption& settings) {
    if (!settings.integration.enabled()) {
        return false;
    }
    if (((std::string)settings.integration.token).empty()) {
        log_dpp("\"Token\" must not be empty. Stopping...", "check_if_empty()", loglevel::ll_error);
        return false;
    }
    else if (((std::string)settings.integration.token).find(",") != std::string::npos) {
        log_dpp("\"Token\" must only contain one token. Stopping...", "check_if_empty()", loglevel::ll_error);
        return false;
    }
    return true;
}

void Handler::create_commands(cluster& bot) {
    command_map.emplace("ping", SlashCommand {
        slashcommand("ping", "Ping pong!", bot.me.id),
        [](const slashcommand_t& event) {
            event.reply("Pong! :ping_pong:");
        }
    });

    command_map.emplace("about", SlashCommand {
        slashcommand("about", "Some info about me!", bot.me.id),
        [](const slashcommand_t& event) {
            embed embed;
            embed.set_color((uint32_t)color).set_title("Here's a little bit about me!");

            int counts = 0;
            if (!Utility::user_counts.empty()) {
                for (auto& count : Utility::user_counts) {
                    counts += count.second;
                }
            }

            embed.add_field("Owner", owner.format_username() + "(" + std::to_string(owner.id) + ")");
            embed.add_field("Guilds", std::to_string(Utility::user_counts.size()));
            embed.add_field("Users", std::to_string(counts));
            embed.add_field("Powered By", std::format("{} {} ([GitHub]({}/About)/[Discord]({}))", PROGRAM_NAME, PROGRAM_VERSION, PROJECT_GITHUB_URL, DISCORD_LINK_URL));

            message message;
            message.add_embed(embed);
            event.reply(message);
        }
    });

    command_map.emplace("hi", SlashCommand {
        slashcommand("hi", "Hi!", bot.me.id),
        [](const slashcommand_t& event) {
            user usr = event.command.get_issuing_user();
            event.reply(usr.get_mention() + " " + (std::string)GlobalSettings::instance().DISCORD.integration.hello_message);
        }
    });

    command_map.emplace("resetserial", SlashCommand {
        slashcommand("resetserial", "Reset the serial connection.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0)),
        [](const slashcommand_t& event) {
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            std::string response = Integration::reset_serial(id);

            if (!response.empty()) {
                event.reply(response);
                return;
            }

            response = std::format("Reset the serial connection for console ID {}.", id);
            event.reply(response);
        }
    });

    command_map.emplace("resetcamera", SlashCommand {
        slashcommand("resetcamera", "Reset the camera.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0)),
        [](const slashcommand_t& event) {
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            std::string response = Integration::reset_camera(id);

            if (!response.empty()) {
                event.reply(response);
                return;
            }

            response = std::format("Reset the camera for console ID {}.", id);
            event.reply(response);
        }
    });

    command_map.emplace("start", SlashCommand {
        slashcommand("start", "Start the currently selected program.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0)),
        [](const slashcommand_t& event) {
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            std::string response = Integration::start_program(id);

            if (!response.empty()) {
                event.reply(response);
                return;
            }

            response = std::format("Started the program for console ID {}.", id);
            event.reply(response);
        }
    });

    command_map.emplace("stop", SlashCommand {
        slashcommand("stop", "Stop the currently running program.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0)),
        [](const slashcommand_t& event) {
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            std::string response = Integration::stop_program(id);

            if (!response.empty()) {
                event.reply(response);
                return;
            }

            response = std::format("Stopped the program for console ID {}.", id);
            event.reply(response);
        }
    });

    command_map.emplace("status", SlashCommand {
        slashcommand("status", "Show program status.", bot.me.id),
        [](const slashcommand_t& event) {
            embed emb;
            emb.set_color((uint32_t)color).set_title("Program Status").set_description(Integration::status());

            message msg;
            msg.add_embed(emb);
            event.reply(msg);
        }
    });

    command_map.emplace("click", SlashCommand {
        slashcommand("click", "Click a button for the specified console.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0))
        .add_option(command_option(command_option_type::co_integer, "button", "Switch console button.", true)
            .add_choice(command_option_choice("Y", 0))
            .add_choice(command_option_choice("B", 1))
            .add_choice(command_option_choice("A", 2))
            .add_choice(command_option_choice("X", 3))
            .add_choice(command_option_choice("L", 4))
            .add_choice(command_option_choice("R", 5))
            .add_choice(command_option_choice("ZL", 6))
            .add_choice(command_option_choice("ZR", 7))
            .add_choice(command_option_choice("Minus", 8))
            .add_choice(command_option_choice("Plus", 9))
            .add_choice(command_option_choice("LStick", 10))
            .add_choice(command_option_choice("RStick", 11))
            .add_choice(command_option_choice("Home", 12))
            .add_choice(command_option_choice("Capture", 13))
            .add_choice(command_option_choice("DUP", 14))
            .add_choice(command_option_choice("DDOWN", 15))
            .add_choice(command_option_choice("DLEFT", 16))
            .add_choice(command_option_choice("DRIGHT", 17))),
        [](const slashcommand_t& event) {
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            uint16_t bt = std::get<int64_t>(event.get_parameter("button"));
            uint16_t button = Utility::get_button(bt);

            if (bt > 13) {
                Integration::press_dpad(id, button, 50);
            }
            else {
                Integration::press_button(id, button, 50);
            }

            std::string name = command_map[event.command.get_command_name()].command.options.at(1).choices.at(bt).name;
            std::string response = std::format("Console ID {} pressed button {}.", id, name);
            event.reply(response);
        }
    });

    command_map.emplace("joystick", SlashCommand {
        slashcommand("joystick", "Move a joystick for the specified console.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0))
        .add_option(command_option(command_option_type::co_string, "stick", "Switch console joystick.", true)
            .add_choice(command_option_choice("Left Stick", "LStick"))
            .add_choice(command_option_choice("Right Stick", "RStick")))
        .add_option(command_option(command_option_type::co_integer, "magnitude_x", "Movement amount in the horizontal direction. \"Left\" is 0, \"right\" is 255, \"neutral\" is 127.", true)
            .set_min_value(0).set_max_value(255))
        .add_option(command_option(command_option_type::co_integer, "magnitude_y", "Movement amount in the vertical direction. \"Down\" is 0, \"up\" is 255, \"neutral\" is 127.", true)
            .set_min_value(0).set_max_value(255))
        .add_option(command_option(command_option_type::co_integer, "ticks", "How long to hold the stick for, in ticks.", true)
            .set_min_value(0)),
        [](const slashcommand_t& event) {
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            std::string stick = std::get<std::string>(event.get_parameter("stick"));
            uint8_t x = std::get<int64_t>(event.get_parameter("magnitude_x"));
            uint8_t y = std::get<int64_t>(event.get_parameter("magnitude_y"));
            uint16_t ticks = std::get<int64_t>(event.get_parameter("ticks"));

            if (stick == "LStick") {
                Integration::press_left_joystick(id, x, y, ticks);
            }
            else {
                Integration::press_left_joystick(id, x, y, ticks);
            }

            std::string response = std::format("Console ID {} moved {} (X: {}, Y: {}) for {} ticks.", id, stick, x, y, ticks);
            event.reply(response);
        }
    });

    command_map.emplace("screenshot", SlashCommand {
        slashcommand("screenshot", "Take and upload a screenshot from the specified console.", bot.me.id)
        .add_permission(command_permission(owner.id, command_permission_type::cpt_user, true))
        .add_option(command_option(command_option_type::co_integer, "id", "Console ID. Find yours by using the \"status\" command.", true)
            .set_min_value(0))
        .add_option(command_option(command_option_type::co_integer, "format", "Image format.", true)
            .add_choice(command_option_choice("PNG", 0))
            .add_choice(command_option_choice("JPG", 1))),
        [this](const slashcommand_t& event) {
            // Serves as an ack because we have to reply within 3 seconds.
            event.thinking();
            uint64_t id = std::get<int64_t>(event.get_parameter("id"));
            uint8_t format = std::get<int64_t>(event.get_parameter("format"));

            std::string path;
            if (format == 0) {
                path = "screenshot_slash.png";
            }
            else {
                path = "screenshot_slash.jpg";
            }

            std::string response = Integration::screenshot(id, path.c_str());
            if (!response.empty()) {
                event.edit_response(response);
                return;
            }
            else {
                std::shared_ptr<PendingFileSend> file(new PendingFileSend(path, true));
                embed_footer footer;
                footer.set_text(std::format("Console ID: {}", id));

                embed emb;
                emb.set_title("Program Screenshot").set_color((uint32_t)color).set_footer(footer);
                Handler::update_response(event, emb, "", std::move(file));
            }
        }
    });
}


}
}
}
#endif
