/*  Process Priority Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessPriorityOption_H
#define PokemonAutomation_ProcessPriorityOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Environment/Environment.h"

namespace PokemonAutomation{




class ThreadPriorityOption : public EnumDropdownOption<ThreadPriority>{
public:
    ThreadPriorityOption(std::string label, ThreadPriority default_priority)
        : EnumDropdownOption<ThreadPriority>(
            std::move(label),
            PRIORITY_DATABASE(),
            LockMode::LOCK_WHILE_RUNNING,
            default_priority
        )
    {}
    void set_on_this_thread() const{
        set_thread_priority(*this);
    }
};




}
#endif
