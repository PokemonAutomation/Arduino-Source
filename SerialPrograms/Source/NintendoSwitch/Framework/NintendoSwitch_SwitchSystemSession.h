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

#include "ConsoleInfra/ConsoleSystemSession.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemOption;




class SwitchSystemSession final : public UiState<SwitchSystemSession, ConsoleInfra::ConsoleSystemSession>
{
public:
    virtual bool try_shutdown() noexcept override;
    ~SwitchSystemSession();
    SwitchSystemSession(
        SwitchSystemOption& option,
        size_t console_number,
        std::optional<uint64_t> program_id
    );


public:
    ConsoleModelCell& console_type(){ return m_option.m_console_type; }


private:
    SwitchSystemOption& m_option;
    std::optional<uint64_t> m_console_id;
};



}
}
#endif
