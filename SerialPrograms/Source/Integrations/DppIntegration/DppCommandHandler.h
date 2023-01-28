#pragma once
#ifndef DPP_HANDLER_H
#define DPP_HANDLER_H

#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppUtility.h>
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Notifications/MessageAttachment.h"
#include "Integrations/DiscordSettingsOption.h"

namespace PokemonAutomation {
	namespace Integration {
		namespace DppCommandHandler {
			class Handler : DppUtility::Utility {
			public:
				Handler() {}

			private:
				struct SlashCommand {
					dpp::slashcommand command;
					std::function<void(const dpp::slashcommand_t&)> func;
				};

				bool m_initialized = false;
				std::mutex m_count_lock;
				static std::map<std::string, SlashCommand> command_map;
				static dpp::user owner;
				static Color color;

			protected:
				void initialize(dpp::cluster& bot);
				bool check_if_empty(const DiscordSettingsOption& settings);
				void log_dpp(const std::string& message, const std::string& identity, const dpp::loglevel& ll);
				void send_message(
					dpp::cluster& bot,
					dpp::embed& embed,
					const std::string& channel,
					std::chrono::milliseconds delay,
					const std::string& msg,
					std::shared_ptr<PendingFileSend> fil
				);

			private:
				void create_commands(dpp::cluster& bot);
			};
		}
	}
}
#endif
