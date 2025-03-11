/*  Program Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A singleton class that keeps track of all live programs and handles.
 *  This allows Discord integration to safely interface with programs.
 *
 */

#ifndef PokemonAutomation_ProgramTrackerInterfaces_H
#define PokemonAutomation_ProgramTrackerInterfaces_H

#include "Common/Cpp/Time.h"
#include "CommonFramework/Globals.h"

namespace PokemonAutomation{

class VideoFeed;
class AudioFeed;
class ControllerSession;
class BotBaseHandle;


class TrackableConsole{
public:
    virtual VideoFeed& video() = 0;
    virtual AudioFeed& audio() = 0;
    virtual ControllerSession& controller() = 0;
};

class TrackableProgram{
public:
    virtual const std::string& identifier() const = 0;
    virtual WallClock timestamp() const = 0;
    virtual ProgramState current_state() const = 0;
    virtual std::string current_stats() const = 0;

    virtual void async_start() = 0;
    virtual void async_stop() = 0;
};




}
#endif
