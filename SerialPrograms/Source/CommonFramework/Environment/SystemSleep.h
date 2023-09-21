/*  System Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SystemSleep_H
#define PokemonAutomation_SystemSleep_H

#if defined(__APPLE__)
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

namespace PokemonAutomation{


#if defined(__APPLE__)

// Call OS API to prevent screen saver from running and OS from going to sleep.
// Useful for running some programs like PokemonSV_VideoFastCodeEntry that require
// the screen to be constantly on.
class SystemSleepController{
public:
    SystemSleepController();
    ~SystemSleepController();

    // Disable/Enable screen saver and OS sleep.
    // Return whether the setting is successful.
    bool prevent_sleep(bool prevent);

private:
    bool disable_sleep();
    bool enable_sleep();

    IOReturn m_prevention_succeeded;
    IOPMAssertionID m_session_id;
};

#endif

}
#endif
