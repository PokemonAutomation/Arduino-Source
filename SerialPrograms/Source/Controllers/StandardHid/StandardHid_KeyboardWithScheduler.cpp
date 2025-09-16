/*  HID Keyboard Controller (With Scheduler)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "StandardHid_KeyboardWithScheduler.h"

namespace PokemonAutomation{
namespace StandardHid{



void KeyboardControllerWithScheduler::issue_key(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    KeyboardKey key
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        m_scheduler.issue_to_resource(
            schedule, std::make_unique<KeyboardCommand>(key, m_seqnum++),
            delay, hold, cooldown
        );
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        m_logger.log(
            "issue_key(): " + std::to_string((size_t)key) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}
void KeyboardControllerWithScheduler::issue_keys(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    const std::vector<KeyboardKey>& keys
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        for (KeyboardKey key : keys){
            m_scheduler.issue_wait_for_resource(schedule, (size_t)key);
        }
        for (KeyboardKey key : keys){
            m_scheduler.issue_to_resource(
                schedule, std::make_unique<KeyboardCommand>(key, m_seqnum++),
                WallDuration::zero(), hold, cooldown
            );
        }
        m_scheduler.issue_nop(schedule, delay);
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        //  Do not log the contents of the command due to privacy concerns.
        //  (people entering passwords)
        m_logger.log(
            "issue_keys(): "
            "delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}























}
}
