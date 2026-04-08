/*  HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "HTTP.h"
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation
{
    namespace Server
    {
        HTTPServer::HTTPServer()
            : m_logger([]() -> TaggedLogger& {
              static TaggedLogger logger(global_logger_raw(), "HTTP");
              return logger;
            }())
            , m_server(new QHttpServer())
            , m_tcpServer(nullptr)
        {}

        HTTPServer::~HTTPServer()
        {
            stop();
            delete m_server;
        }

        bool HTTPServer::start(quint16 port)
        {
            if (m_tcpServer)
            {
                m_logger.log("HTTP Server already running", COLOR_YELLOW);
                return false;
            }

            // Init HTTP Server
            m_tcpServer = new QTcpServer();

            if (!m_tcpServer->listen(QHostAddress::Any, port))
            {
                m_logger.log("Failed to start TCP server", COLOR_RED);
                return false;
            }
            if (!m_server->bind(m_tcpServer))
            {
                m_logger.log("Failed to bind TCP server", COLOR_RED);
                return false;
            }

            m_logger.log(("HTTP Server started on port " + std::to_string(port)).c_str(), COLOR_GREEN);
            return true;
        }

        void HTTPServer::stop()
        {
            if (m_tcpServer)
            {
                m_tcpServer->close();
                delete m_tcpServer;
                m_tcpServer = nullptr;
                m_logger.log("HTTP Server stopped", COLOR_GREEN);
            }
        }

        void HTTPServer::addRoute(
            const QString& path,
            QHttpServerRequest::Method method,
            std::function<void(const QHttpServerRequest&, QHttpServerResponder&)> handler
        )
        {
            if (!m_server)
            {
                m_logger.log("Failed to add route, HTTP Server not initialised", COLOR_RED);
                return;
            }

            m_server->route(path, method, [handler](const QHttpServerRequest& req, QHttpServerResponder& res)
            {
                handler(req, res);
            });
        }
    }
}
