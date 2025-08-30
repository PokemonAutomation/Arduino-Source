#ifdef PA_SOCIAL_SDK
#define DISCORDPP_IMPLEMENTATION

#include <thread>
#include "DiscordSocial.h"
#include "CommonFramework/Logging/Logger.h"
#include "Integrations/ProgramTracker.h"
#include "../../CommonFramework/GlobalSettingsPanel.h"

using namespace discordpp;
namespace PokemonAutomation{
namespace Integration{
namespace DiscordSocialSDK{

DiscordSocial& DiscordSocial::instance(){
    static DiscordSocial instance;
    return instance;
}

void DiscordSocial::run(){
    m_client = std::make_shared<Client>();
    if (!m_client){
        log("Failed to initialize DiscordSocialSDK.", "run()", LoggingSeverity::Error);
        m_client.reset();
        return;
    }

    m_client->SetApplicationId(m_app_id);
    m_client->AddLogCallback([&](auto message, auto severity){
        log(message, "Internal", severity);
    }, m_log_level);

    std::thread(&DiscordSocial::thread_loop, this).detach();
}

void DiscordSocial::thread_loop(){
    log("Starting Discord Rich Presence update thread...", "thread_loop()", LoggingSeverity::Info);
    m_running.store(true, std::memory_order_release);
    m_activity = create_activity();
    m_client->UpdateRichPresence(m_activity, [&](ClientResult result){
        if (!result.Successful()){
            log("Rich Presence initial update failed.", "thread_loop()", LoggingSeverity::Warning);
        }
    });

    while (m_running.load(std::memory_order_relaxed)){
        try{
            if (!m_running.load(std::memory_order_relaxed) || !GlobalSettings::instance().RICH_PRESENCE){
                break;
            }

            RunCallbacks();
            update_rich_presence();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }catch (const std::exception& e){
            log("Exception: " + std::string(e.what()), "thread_loop()", LoggingSeverity::Error);
            break;
        }
    }

    log("Discord Rich Presence update thread exiting...", "thread_loop()", LoggingSeverity::Info);
    m_running.store(false, std::memory_order_release);
    m_client.reset();
}

void DiscordSocial::update_rich_presence(){
    try{
        std::string details = m_activity.Details().value();
        std::string state = m_activity.State().value();

        discordpp::ActivityAssets assets{};
        assets.SetLargeImage("rotom");
        assets.SetLargeText("Pok\u00E9mon Automation");

        for (const auto& item : ProgramTracker::instance().all_programs()){
            switch (item.second.state) {
                case ProgramState::RUNNING:{
                    assets.SetSmallImage("running");
                    assets.SetSmallText("Running");
                    state = "Running"; 
                    m_activity.SetAssets(assets); break;
                }
                case ProgramState::STOPPED:{
                    assets.SetSmallImage("stopped");
                    assets.SetSmallText("Stopped");
                    state = "Stopped";
                    m_activity.SetAssets(assets); break;
                }
                default: {
                    assets.SetSmallImage("idle");
                    assets.SetSmallText("Idle");
                    state = "Idle";
                    m_activity.SetAssets(assets); break;
                }
            }

            if (item.second.program_name != details){
                details = item.second.program_name;
            }

            m_timestamps.SetStart(std::chrono::duration_cast<std::chrono::seconds>(item.second.start_time.time_since_epoch()).count());
        }

        m_activity.SetTimestamps(m_timestamps);
        m_activity.SetDetails(details);
        m_activity.SetState(state);
        m_client->UpdateRichPresence(m_activity, [&](ClientResult result){
            if (!result.Successful()){
                log("Rich Presence update failed.", "update_rp()", LoggingSeverity::Warning);
            }
        });
    }catch (const std::exception& e){
        log("Exception: " + std::string(e.what()), "update_rp()", LoggingSeverity::Error);
        m_running.store(false, std::memory_order_release);
    }
}

Logger& DiscordSocial::logger(){
    static TaggedLogger logger(global_logger_raw(), "DiscordSocialSDK");
    return logger;
}

void DiscordSocial::log(const std::string& message, const std::string& identity, const LoggingSeverity& severity){
    if (severity < m_log_level){
        return;
    }

    std::string log = identity + ": " + message;
    Color color;
    switch (severity){
        case LoggingSeverity::Verbose: color = COLOR_GRAY; break;
        case LoggingSeverity::Info: color = COLOR_CYAN; break;
        case LoggingSeverity::Warning: color = COLOR_ORANGE; break;
        case LoggingSeverity::Error: color = COLOR_MAGENTA; break;
        default: color = COLOR_PURPLE; break;
    };

    logger().log(log, color);
}

Activity DiscordSocial::create_activity(){
    Activity activity{};
    activity.SetType(ActivityTypes::Playing);
    activity.SetDetails("ComputerControl: Preparing to automate...");
    activity.SetState("Idle");

    ActivityButton button1{};
    button1.SetLabel("Download from GitHub");
    button1.SetUrl("https://github.com/PokemonAutomation/ComputerControl/releases/latest");
    activity.AddButton(button1);

    ActivityButton button2{};
    button2.SetLabel("Join our Discord");
    button2.SetUrl("https://discord.gg/pokemonautomation");
    activity.AddButton(button2);

    discordpp::ActivityAssets assets{};
    assets.SetLargeImage("rotom");
    assets.SetLargeText("Pok\u00E9mon Automation");
    assets.SetSmallImage("idle");
    assets.SetSmallText("Idle");
    activity.Assets().emplace(assets);
    return activity;
}


}
}
}
#endif
