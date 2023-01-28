#pragma once
#ifndef DPP_CLIENT_H
#define DPP_CLIENT_H

#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppCommandHandler.h>
#include "CommonFramework/Notifications/MessageAttachment.h"

namespace PokemonAutomation {
	namespace Integration {
		namespace DppClient {

			class Client : protected DppCommandHandler::Handler {
			public:
				Client() {}
				static Client& instance();

			private:
				std::unique_ptr<dpp::cluster> m_bot = nullptr;
				bool m_is_connected = false;
				std::mutex m_connect_lock;

			public:
				void connect();
				bool is_running();
				void disconnect();
				void send_message_dpp(
					bool should_ping,
					const Color& color,
					const std::vector<std::string>& tags,
					const JsonObject& json_obj,
					const std::string& message,
					std::shared_ptr<PendingFileSend> file
				);

			private:
				void run(const std::string& token);
			};
		}
	}
}
#endif
