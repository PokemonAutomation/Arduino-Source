/*  Thread Pool Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_ThreadPoolOption_H
#define PokemonAutomation_Options_ThreadPoolOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Environment/ProcessPriorityOption.h"

namespace PokemonAutomation{


class ThreadPoolOption : public GroupOption{
public:
    ThreadPoolOption(
        std::string label,
        std::string description,
        ThreadPriority default_priority,
        double default_max_thread_ratio = 1.0
    );

    virtual void load_json(const JsonValue& json) override;

private:
    const size_t m_default_max_threads;
    SimpleIntegerOption<size_t> HARDWARE_THREADS;

public:
    StaticTextOption m_description;
    ThreadPriorityOption PRIORITY;
    SimpleIntegerOption<size_t> MAX_THREADS;
};



}
#endif
