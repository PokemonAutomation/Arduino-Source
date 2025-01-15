/*  Video Stream
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoStream_H
#define PokemonAutomation_VideoStream_H

namespace PokemonAutomation{

class Logger;
class AudioFeed;
class VideoFeed;
class StreamHistorySession;


struct VideoStream{
    Logger& logger;
    AudioFeed& audio;
    VideoFeed& video;
    const StreamHistorySession& video_history;
};



}
#endif
