/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of an entire Switch system.
 *
 *    - Serial Port
 *    - Camera
 *    - Audio
 *    - Video Overlay
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemSession_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemSession_H

#include "Integrations/ProgramTrackerInterfaces.h"
#include "ConsoleInfra/ConsoleSystemSession.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemOption;




class SwitchSystemSession final : public TrackableConsole, public ConsoleInfra::ConsoleSystemSession{
public:
    virtual bool try_shutdown() noexcept override;
    ~SwitchSystemSession();
    SwitchSystemSession(
        SwitchSystemOption& option,
        uint64_t program_id,
        size_t console_number
    );


public:
    virtual VideoFeed& video_feed() override{ return video(); }
    virtual AudioFeed& audio_feed() override{ return audio(); }
    virtual ControllerSession& controller() override{ return ConsoleSystemSession::controller(0); };
    ConsoleModelCell& console_type(){ return m_option.m_console_type; }


private:
    //  Globally unique ID.
    uint64_t m_console_id = 0;
    SwitchSystemOption& m_option;
};



}
}
#endif
