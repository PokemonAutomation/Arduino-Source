/*  WebSocket Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "WebSocket.h"
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation
{
    namespace Server
    {
        WSServer::WSServer(QObject* parent)
            : QObject(parent)
            , m_logger([]() -> TaggedLogger& {
                  static TaggedLogger logger(global_logger_raw(), "WS");
                  return logger;
              }())
            , m_server(nullptr)
        {}

        WSServer::~WSServer()
        {
            stop();
        }

        // Start WebSocket Server
        bool WSServer::start(quint16 port)
        {
            if (m_server) return true; // Already running

            m_server = new QWebSocketServer(
                QStringLiteral("SerialPrograms WebSocket Server"),
                QWebSocketServer::NonSecureMode, // Change later?
                this
            );

            // Attempt to bind port
            if (!m_server->listen(QHostAddress::Any, port))
            {
                m_logger.log("Failed to start WebSocket Server", COLOR_RED);
                return false;
            }

            connect(m_server, &QWebSocketServer::newConnection,
                this, &WSServer::clientConnection);

            m_logger.log(("WebSocket Server started on port " + std::to_string(port)).c_str(), COLOR_GREEN);
            emit serverStarted(port);
            return true;
        }

        // Stop WebSocket Server
        void WSServer::stop()
        {
            if (!m_server) return;

            // Clean up clients
            for (auto* client : m_clients)
            {
                client->close();
                client->deleteLater();
            }
            m_clients.clear();

            m_server->close();
            m_server->deleteLater();
            m_server = nullptr;

            m_logger.log("WebSocket Server stopped", COLOR_GREEN);
            emit serverStopped();
        }

        // Called when a new client connects
        void WSServer::clientConnection()
        {
            QWebSocket* client = m_server->nextPendingConnection();
            if (!client) return;

            m_clients.append(client);
            connect(client, &QWebSocket::disconnected, this,
                &WSServer::clientDisconnection);

            m_logger.log("WebSocket client connected", COLOR_GREEN);

            emit clientConnected(client);
        }

        // Called when a client disconnects
        void WSServer::clientDisconnection()
        {
            QWebSocket* client = qobject_cast<QWebSocket*>(sender());
            if (!client) return;

            m_clients.removeAll(client);
            client->deleteLater();

            m_logger.log("WebSocket client disconnected", COLOR_GREEN);

            emit clientDisconnected(client);
        }

        // Handle text messages from clients
        void WSServer::handleMessage(const QString& message)
        {
            QWebSocket* client = qobject_cast<QWebSocket*>(sender());
            if (!client) return;

            emit messageReceived(client, message);
        }

        // Handle binary messages from clients
        void WSServer::handleBinary(const QByteArray& data)
        {
            QWebSocket* client = qobject_cast<QWebSocket*>(sender());
            if (!client) return;

            emit binaryReceived(client, data);
        }

        // Send text message to a specific client
        void WSServer::send_message(QWebSocket* client, const QString& message)
        {
            if (!client) return;

            QMetaObject::invokeMethod(client, [client, message]()
            {
                if (client->isValid())
                    client->sendTextMessage(message);
            }, Qt::QueuedConnection);
        }

        // Send binary message to a specific client
        void WSServer::send_binary(QWebSocket* client, const QByteArray& data)
        {
            if (!client) return;

            QMetaObject::invokeMethod(client, [client, data]()
            {
                if (client->isValid())
                    client->sendBinaryMessage(data);
            }, Qt::QueuedConnection);
        }

        // Send text message to all clients
        void WSServer::send_message(const QString& message)
        {
            for (auto* client : m_clients)
            {
                QMetaObject::invokeMethod(client, [client, message]() {
                    if (client->isValid())
                        client->sendTextMessage(message);
                }, Qt::QueuedConnection);
            }
        }

        // Send binary message to all clients
        void WSServer::send_binary(const QByteArray& data)
        {
            for (auto* client : m_clients)
            {
                QMetaObject::invokeMethod(client, [client, data]() {
                    if (client->isValid())
                        client->sendBinaryMessage(data);
                }, Qt::QueuedConnection);
            }
        }
    }
}
