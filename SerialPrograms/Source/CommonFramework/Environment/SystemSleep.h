/*  System Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SystemSleep_H
#define PokemonAutomation_SystemSleep_H

#include <memory>


#if _WIN32
#define PA_ENABLE_SLEEP_SUPPRESS
#define PA_ENABLE_SLEEP_SUPPRESS_NO_SLEEP
#endif
#if __APPLE__
#define PA_ENABLE_SLEEP_SUPPRESS
#endif


namespace PokemonAutomation{


//  Call OS API to prevent screen saver from running and OS from going to sleep.
//  Useful for running some programs like PokemonSV_VideoFastCodeEntry that require
//  the screen to be constantly on.
class SystemSleepController{
    SystemSleepController();
    ~SystemSleepController();

public:
    static SystemSleepController& instance(){
        static SystemSleepController controller;
        return controller;
    }

    //  Push: Add a request for this type of sleep-disable.
    //  Pop: Remove a request for this type of sleep-disable.
    //  The sleep-disable will be active as long as there is at least one
    //  request is active for that type.

    //  Keep the screen on and prevent sleep.
    void push_screen_on();
    void pop_screen_on();

    //  Allow the screen to turn off, but don't sleep.
    void push_no_sleep();
    void pop_no_sleep();

private:
    struct InternalController;
    std::unique_ptr<InternalController> m_data;
};




enum class SleepSuppress{
    NONE,
    NO_SLEEP,
    SCREEN_ON,
};


class SleepSuppressScope{
public:
    SleepSuppressScope(SleepSuppress mode){
        set(mode);
    }
    ~SleepSuppressScope(){
        clear();
    }
    SleepSuppressScope(const SleepSuppressScope&) = delete;
    void operator=(const SleepSuppressScope&) = delete;

    void clear(){
        switch (m_mode){
        case SleepSuppress::NONE:
            break;
        case SleepSuppress::NO_SLEEP:
            SystemSleepController::instance().pop_no_sleep();
            break;
        case SleepSuppress::SCREEN_ON:
            SystemSleepController::instance().pop_screen_on();
            break;
        }
            m_mode = SleepSuppress::NONE;
    }
    void operator=(SleepSuppress mode){
        clear();
        set(mode);
    }

private:
    void set(SleepSuppress mode){
        switch (mode){
        case SleepSuppress::NONE:
            break;
        case SleepSuppress::NO_SLEEP:
            SystemSleepController::instance().push_no_sleep();
            break;
        case SleepSuppress::SCREEN_ON:
            SystemSleepController::instance().push_screen_on();
            break;
        }
        m_mode = mode;
    }

private:
    SleepSuppress m_mode;
};




}
#endif
