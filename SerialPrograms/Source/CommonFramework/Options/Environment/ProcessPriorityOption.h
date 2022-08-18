/*  Process Priority Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessPriorityOption_H
#define PokemonAutomation_ProcessPriorityOption_H

#include "Common/Cpp/Options/DropdownOption.h"

class QThread;

namespace PokemonAutomation{


class ProcessPriorityOption : public DropdownOption{
public:
    ProcessPriorityOption();
    virtual bool set_index(size_t index) override;
    void update_priority_to_option() const;
};



class ThreadPriorityOption : public DropdownOption{
public:
    ThreadPriorityOption(std::string label, int default_priority);
    void set_on_this_thread() const;
    void set_on_qthread(QThread& thread) const;
};



}
#endif
