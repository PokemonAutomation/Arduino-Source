/*  Controller Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Release every input on a controller and confirm that the device has done it.
 */

#ifndef PokemonAutomation_Controllers_ControllerRelease_H
#define PokemonAutomation_Controllers_ControllerRelease_H

#include "Common/Cpp/Time.h"

namespace PokemonAutomation{

class AbstractController;


//  Cancel every queued command (`cancel_all_commands()`), then wait at most `timeout`
//  for the device to confirm that it is in the neutral state.
//
//  `cancel_all_commands()` only starts the cancellation: it returns as soon as the
//  cancel request is handed to the connection, and the host marks its command
//  queue empty at that moment, so waiting for the queue proves nothing. Instead this
//  queues a 10 ms neutral no-op after the cancel and waits for the device to report
//  that the no-op finished. Commands are delivered in order, so that report means
//  the device has processed the cancel and is holding the neutral state.
//
//  Returns true if confirmed; false on timeout (e.g. the Switch is asleep and the
//  device can't execute commands) or if the controller is not ready.
//  Thread-safe: may be called while another thread is issuing commands or waiting on
//  this controller. The timeout covers waiting for the device; a command that
//  another thread is in the middle of issuing is allowed to finish first.
bool release_all_and_confirm(AbstractController& controller, Milliseconds timeout);


}
#endif
