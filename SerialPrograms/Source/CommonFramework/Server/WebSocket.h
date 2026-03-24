/*  WebSocket Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef SERIALPROGRAMS_WEBSOCKET_H
#define SERIALPROGRAMS_WEBSOCKET_H

#pragma once

#include <qtmetamacros.h>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QObject>
#include "CommonFramework/Logging/Logger.h"


namespace PokemonAutomation
{
    namespace Server
    {
        class WSServer : public QObject
        {
            Q_OBJECT
        public:
            static WSServer& instance()
            {
                static WSServer instance;
                return instance;
            }

            // Start/Stop
            bool start(quint16 port = 8081);
            void stop();

            // Send to all clients
            void send_message(const QString& message);
            void send_binary(const QByteArray& data);

            // Send to a specific client
            void send_message(QWebSocket* client, const QString& message);
            void send_binary(QWebSocket* client, const QByteArray& data);

        signals:
            void clientConnected(QWebSocket* client);
            void clientDisconnected(QWebSocket* client);

            void messageReceived(QWebSocket* client, const QString& message);
            void binaryReceived(QWebSocket* client, const QByteArray& data);

            void serverStarted(quint16 port);
            void serverStopped();

        private slots:
            void clientConnection();
            void clientDisconnection();
            void handleMessage(const QString& message);
            void handleBinary(const QByteArray& data);

        private:
            explicit WSServer(QObject* parent = nullptr);
            ~WSServer();

            Logger& m_logger;
            QWebSocketServer* m_server;
            QList<QWebSocket*> m_clients;
        };
    }
}


#endif //SERIALPROGRAMS_WEBSOCKET_H