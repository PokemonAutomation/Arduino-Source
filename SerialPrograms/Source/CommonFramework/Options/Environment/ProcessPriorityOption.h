/*  Process Priority Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessPriorityOption_H
#define PokemonAutomation_ProcessPriorityOption_H

#include "CommonFramework/Options/EnumDropdownOption.h"

class QThread;

namespace PokemonAutomation{


class ProcessPriorityOption : public EnumDropdownOption{
public:
    ProcessPriorityOption();
    void update_priority_to_option() const;

    virtual ConfigWidget* make_ui(QWidget& parent) override;
};



class ThreadPriorityOption : public EnumDropdownOption{
public:
    ThreadPriorityOption(std::string label, int default_priority);
    void set_on_this_thread() const;
    void set_on_qthread(QThread& thread) const;
};



}
#endif
