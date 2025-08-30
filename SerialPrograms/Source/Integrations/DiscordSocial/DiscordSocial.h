#pragma once
#ifndef DISCORD_SOCIAL_H
#define DISCORD_SOCIAL_H

#include <thread>
#include <atomic>
#include <discord_social_sdk/discordpp.h>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{
namespace Integration{
namespace DiscordSocialSDK{
    class DiscordSocial{
        public:
            DiscordSocial() : m_running(false) {}
            ~DiscordSocial(){
                m_running.store(false, std::memory_order_release);
                m_thread.join();
                if (m_client) m_client.reset();
            }

            static DiscordSocial& instance();
            void run();

        private:
            Logger& logger();
            void log(const std::string& message, const std::string& identity, const discordpp::LoggingSeverity& severity);
            discordpp::Activity create_activity();
            void update_rich_presence();
            void thread_loop();

        private:
            std::thread m_thread;
            std::shared_ptr<discordpp::Client> m_client = nullptr;
            std::atomic<bool> m_running;
            const uint64_t m_app_id = 1406867596585865326;
            discordpp::Activity m_activity;
            discordpp::ActivityTimestamps m_timestamps{};
            discordpp::LoggingSeverity m_log_level = discordpp::LoggingSeverity::Error;
            const std::string m_github_release_latest = "https://github.com/PokemonAutomation/ComputerControl/releases/latest";
    };
}
}
}
#endif
