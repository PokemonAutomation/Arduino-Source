/*  Global Gamepad Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_GAMEPAD

#include <chrono>
#include <mutex>
#include <string>
#include <SDL3/SDL.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "GamepadInput_SDL.h"
#include "GlobalGamepadTracker.h"

namespace PokemonAutomation{

GamepadTracker& global_gamepad_tracker(){
    static GamepadTracker tracker;
    return tracker;
}

GamepadTracker::~GamepadTracker(){
    stop();
}

void GamepadTracker::start(){
    if (m_initialized){
        return;
    }
    // Qt owns the window/event loop. SDL only owns joystick input.
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)){
        global_logger_tagged().log(std::string("Gamepad initialization failed: ") + SDL_GetError());
        return;
    }
    m_initialized = true;
    SDL_SetGamepadEventsEnabled(false);
    SDL_SetJoystickEventsEnabled(false);
    m_stopping.store(false, std::memory_order_release);
    m_thread = GlobalThreadPools::unlimited_realtime().dispatch_now_blocking([this]{
        thread_loop();
    });
}

void GamepadTracker::stop() noexcept{
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<Mutex> lg(m_sleep_lock);
    }
    m_cv.notify_all();
    if (m_thread){
        m_thread.wait_and_ignore_exceptions();
    }
    if (m_initialized){
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        m_initialized = false;
    }
}

void GamepadTracker::clear_state(){
    m_reset.store(true, std::memory_order_release);
}

void GamepadTracker::thread_loop(){
    SDL_Gamepad* pad = nullptr;
    GamepadInputState last;
    GamepadInputGate gate;
    auto refresh = std::chrono::steady_clock::now();
    auto discover = refresh;
    while (!m_stopping.load(std::memory_order_acquire)){
        SDL_UpdateGamepads();
        auto now = std::chrono::steady_clock::now();
        if (pad && !SDL_GamepadConnected(pad)){
            SDL_CloseGamepad(pad);
            pad = nullptr;
            gate.reset();
        }
        if (!pad && now >= discover){
            discover = now + std::chrono::seconds(1);
            int count = 0;
            SDL_JoystickID* ids = SDL_GetGamepads(&count);
            for (int c = 0; c < count && !pad; c++){
                pad = SDL_OpenGamepad(ids[c]);
            }
            SDL_free(ids);
        }
        GamepadInputState current;
        if (pad){
            current = read_gamepad(pad);
        }
        if (m_reset.exchange(false, std::memory_order_acq_rel)){
            gate.reset();
        }
        // Require release after focus/console changes or automation. A held
        // button must never leak to the newly selected console.
        gate.filter(current, !m_listeners.empty());
        if (current != last || (!current.is_neutral() && now >= refresh)){
            try{
                m_listeners.run_method(&ControllerInputListener::run_controller_input, current);
            }catch (Exception& error){
                error.log(global_logger_tagged());
            }
            last = current;
            refresh = now + std::chrono::seconds(1);
        }
        std::unique_lock<Mutex> lock(m_sleep_lock);
        if (!m_stopping.load(std::memory_order_acquire)){
            m_cv.wait_for(lock, std::chrono::milliseconds(8));
        }
    }
    if (pad){
        SDL_CloseGamepad(pad);
    }
}

}

#endif
