/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualController_H
#define PokemonAutomation_NintendoSwitch_VirtualController_H

#include <QKeyEvent>
#include "Controllers/ControllerSession.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch_VirtualControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class VirtualController : public KeyboardInputController{
public:
    VirtualController(
        ControllerSession& session,
        bool allow_commands_while_running
    );
    virtual ~VirtualController();

private:
    virtual std::unique_ptr<ControllerState> make_state() const override;
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) override;
    virtual bool try_stop_commands() override;
    virtual bool try_next_interrupt() override;
    virtual bool try_send_state(const ControllerState& state) override;

private:
    std::map<Qt::Key, ControllerDeltas> m_mapping;
    ControllerSession& m_session;
};




}
}
#endif
