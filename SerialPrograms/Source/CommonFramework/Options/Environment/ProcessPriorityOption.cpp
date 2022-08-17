/*  Process Priority Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/Options/EnumDropdownWidget.h"
#include "CommonFramework/Environment/Environment.h"
#include "ProcessPriorityOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ProcessPriorityOption::ProcessPriorityOption()
    : DropdownOption(
        "<b>Process Priority:</b><br>"
        "Set the priority of this process.<br>"
        "Higher priority may reduce errors due to CPU starvation from background programs. "
        "Lower priority may improve system responsiveness if your computer isn't powerful enough.<br>"
        "On Mac and Linux, you may need to restart the program for this to properly take effect.",
        PRIORITY_MODES, DEFAULT_PRIORITY_INDEX
    )
{}
bool ProcessPriorityOption::set_index(size_t index){
    if (!DropdownOption::set_index(index)){
        return false;
    }
    set_priority_by_index((int)index);
    return true;
}
void ProcessPriorityOption::update_priority_to_option() const{
    if (PRIORITY_MODES.size() > 1){
        set_priority_by_name(current_case());
    }
}






std::vector<std::string> make_thread_priority_list(){
    std::vector<std::string> ret;
    for (int priority = THREAD_PRIORITY_MIN; priority <= THREAD_PRIORITY_MAX; priority++){
        ret.emplace_back(thread_priority_name(priority));
    }
    return ret;
}
ThreadPriorityOption::ThreadPriorityOption(std::string label, int default_priority)
    : DropdownOption(
        std::move(label),
        make_thread_priority_list(),
        clip_priority(default_priority) - THREAD_PRIORITY_MIN
    )
{}
void ThreadPriorityOption::set_on_this_thread() const{
    int priority = (int)current_index() + THREAD_PRIORITY_MIN;
    set_thread_priority(priority);
}
void ThreadPriorityOption::set_on_qthread(QThread& thread) const{
    int priority = (int)current_index() + THREAD_PRIORITY_MIN;
    thread.setPriority(to_qt_priority(priority));
}








}
