/*  HID Keyboard Controller (With Scheduler)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This implements most of the Keyboard API using only the controller
 *  state function. It uses SuperscalarScheduler to do this.
 *
 */

#ifndef PokemonAutomation_StandardHid_KeyboardWithScheduler_H
#define PokemonAutomation_StandardHid_KeyboardWithScheduler_H

#include <set>
#include "Common/Cpp/CancellableScope.h"
#include "ControllerInput/Keyboard/KeyboardHidButtons.h"
#include "Controllers/Schedulers/ControllerWithScheduler.h"

namespace PokemonAutomation{
namespace StandardHid{



struct KeyboardControllerState{
    std::set<KeyboardKey> keys;
};

class KeyboardCommand : public SchedulerResource{
public:
    KeyboardCommand(KeyboardKey key, uint64_t seqnum)
        : SchedulerResource((size_t)key)
        , m_seqnum(seqnum)
    {}
    KeyboardKey key() const{
        return (KeyboardKey)id;
    }
    uint64_t seqnum() const{
        return m_seqnum;
    }

private:
    uint64_t m_seqnum;
};



class KeyboardControllerWithScheduler : public PokemonAutomation::ControllerWithScheduler{
public:
    using PokemonAutomation::ControllerWithScheduler::ControllerWithScheduler;


public:
    //  Superscalar Commands (the "ssf" framework)

    void issue_key(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        KeyboardKey key
    );
    void issue_keys(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const std::vector<KeyboardKey>& keys
    );

private:
    uint64_t m_seqnum = 0;
};




}
}
#endif
