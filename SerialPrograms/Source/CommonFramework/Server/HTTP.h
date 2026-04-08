/*  HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef SERIALPROGRAMS_HTTP_H
#define SERIALPROGRAMS_HTTP_H

#pragma once

#include <QtHttpServer/QtHttpServer>
#include "CommonFramework/Logging/Logger.h"


namespace PokemonAutomation
{
    namespace Server
    {
        class HTTPServer
        {
        public:
            static HTTPServer& instance()
            {
                static HTTPServer instance;
                return instance;
            }

            bool start(quint16 port = 8080);
            void stop();

            void addRoute(
                const QString& path,
                QHttpServerRequest::Method method,
                std::function<void(const QHttpServerRequest&, QHttpServerResponder&)> handler
            );

            template <typename... Args, typename Handler>
            void addRoute(
                const QString& path,
                QHttpServerRequest::Method method,
                Handler&& handler
            )
            {
                if (!m_server)
                {
                    m_logger.log("Failed to add route, HTTP Server not initialised", COLOR_RED);
                    return;
                }

                m_server->route(path, method, std::forward<Handler>(handler));
            }

        private:
            explicit HTTPServer();
            ~HTTPServer();

            Logger& m_logger;
            QHttpServer* m_server;
            QTcpServer* m_tcpServer;
        };
    }
}


#endif
