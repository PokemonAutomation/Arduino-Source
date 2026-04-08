/*  Video WebSocket Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef SERIALPROGRAMS_VIDEOWS_H
#define SERIALPROGRAMS_VIDEOWS_H

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/Server/WebSocket.h"

namespace PokemonAutomation
{
    namespace Server
    {
        class VideoWSServer : public VideoFrameListener
        {
        public:
            static VideoWSServer& instance()
            {
                static VideoWSServer instance;
                return instance;
            }

            // Send frames to all connected clients
            virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override
            {
                if (WSServer::instance().hasClients())
                {
                    QByteArray frameData = frame_to_jpeg(*frame);
                    WSServer::instance().send_binary(frameData);
                }
            }

        private:
            VideoWSServer() = default;
        };
    }
}

#endif
