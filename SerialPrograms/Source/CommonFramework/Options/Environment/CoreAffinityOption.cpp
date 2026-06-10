/* Core Affinity Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32
#include <algorithm>
#include <atomic>
//#include <thread>
#include <Windows.h>
#include <processtopologyapi.h>
#include <tlhelp32.h>
#include "Common/Cpp/SparseRegion.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Options/StaticTextOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Logging/Logger.h"
#include "CoreAffinityOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class CoreAffinityOption::Internal : public ConfigOption::Listener{
public:
    Internal(CoreAffinityOption& parent)
        : m_parent(parent)
        , m_processor_groups(GetActiveProcessorGroupCount())
        , m_text("<font color=\"red\">Core affinity is not available if there are multiple processor groups.</font>")
        , m_cores(
            false,
            "<b>Allowed Cores:</b>",
            LockMode::LOCK_WHILE_RUNNING,
            generate_default_core_string(),
            "4-8"
        )
        , m_cached_affinity_mask(get_all_core_mask())
    {
        if (m_processor_groups <= 1){
            m_text.set_visibility(ConfigOptionState::HIDDEN);
        }else{
            m_text.set_visibility(ConfigOptionState::ENABLED);
        }
        parent.add_listener(*this);
        m_cores.add_listener(*this);
    }
    ~Internal(){
        m_cores.remove_listener(*this);
        m_parent.remove_listener(*this);
    }

    bool is_enabled() const{
        return m_processor_groups <= 1;
    }

    static size_t get_all_core_mask(){
        size_t cores = GetActiveProcessorCount(0) - 1;
        if (cores >= CHAR_BIT * sizeof(size_t)){
            return (size_t)-1;
        }
        return ((size_t)1 << GetActiveProcessorCount(0)) - 1;
    }
    std::string generate_default_core_string() const{
        size_t max = 0;
        for (size_t group = 0; group < m_processor_groups; group++){
            max = std::max<size_t>(max, GetActiveProcessorCount((WORD)group) - 1);
        }
        return "0-" + std::to_string(max);
    }

    void update_cached_mask(){
        if (m_processor_groups >= 2){
            return;
        }

        size_t core_count = GetActiveProcessorCount(0);
        size_t mask = 0;

        SparseRegion<size_t> ranges(m_cores, true);
        for (const auto& range : ranges){
            size_t stop = std::min<size_t>(range.second, core_count);
            for (size_t core = range.first; core < stop; core++){
                mask |= (size_t)1 << core;
            }
        }

        m_cached_affinity_mask.store(mask, std::memory_order_release);
//        cout << "ranges = " << ranges.tostr(true) << endl;
        m_cores.set(ranges.tostr(true));
    }

    void set_on_current_thread(){
        if (m_processor_groups >= 2){
            return;
        }

        if (SetThreadAffinityMask(GetCurrentThread(), m_cached_affinity_mask)){
            return;
        }
        global_logger_tagged().log("Failed to set core affinity.", COLOR_RED);
    }

    virtual void on_config_value_changed(void* object) override{
//        cout << "on_config_value_changed(): " << object << endl;

        if (m_processor_groups >= 2){
            return;
        }
        if (m_parent.enabled()){
            update_cached_mask();
        }else{
            m_cached_affinity_mask.store(get_all_core_mask(), std::memory_order_release);
        }

//        cout << "m_cached_affinity_mask = " << m_cached_affinity_mask.load(std::memory_order_acquire) << endl;
        if (SetProcessAffinityMask(
            GetCurrentProcess(),
            m_cached_affinity_mask.load(std::memory_order_acquire)
        )){
            return;
        }
        global_logger_tagged().log("Failed to set process fore affinity.", COLOR_RED);
    }


public:
    CoreAffinityOption& m_parent;
    size_t m_processor_groups;

    StaticTextOption m_text;
    StringOption m_cores;

    std::atomic<size_t> m_cached_affinity_mask;
};




CoreAffinityOption::CoreAffinityOption()
    : GroupOption(
        "Core Affinity",
        LockMode::LOCK_WHILE_RUNNING,
        EnableMode::DEFAULT_DISABLED,
        true
    )
    , m_internal(CONSTRUCT_TOKEN, *this)
{
    PA_ADD_STATIC(m_internal->m_text);
    add_option(m_internal->m_cores, "Cores");

    if (!m_internal->is_enabled()){
        this->set_visibility(ConfigOptionState::DISABLED);
    }
}
CoreAffinityOption::~CoreAffinityOption() = default;














}
#endif
