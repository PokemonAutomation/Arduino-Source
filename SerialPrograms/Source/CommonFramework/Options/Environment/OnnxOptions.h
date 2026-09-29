/* ONNX Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OnnxOptions_H
#define PokemonAutomation_OnnxOptions_H

#include <thread>
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/StaticGlobals.h"
#include "ProcessPriorityOption.h"

namespace PokemonAutomation{


class OnnxOptions : public GroupOption{
public:
    OnnxOptions()
        : GroupOption(
            "ONNX Library",
            LockMode::UNLOCK_WHILE_RUNNING,
            EnableMode::ALWAYS_ENABLED,
            true
        )
        , m_default_max_threads(
            std::max(
                (int)std::thread::hardware_concurrency() - 2,
                1
            )
        )
        , HARDWARE_THREADS(
            "<b>Hardware Threads:</b>",
            LockMode::UNLOCK_WHILE_RUNNING,
            std::thread::hardware_concurrency(),
            1,
            std::numeric_limits<decltype(std::thread::hardware_concurrency())>::max()
        )
        , m_description("ONNX Library options. All changes require restarting the program.")
        , USE_GPU(
            "<b>Use GPU Acceleration:</b><br>"
            "Use the GPU by default for ONNX. Will fall-back to CPU if using the GPU fails.<br>"
            "<font color=\"red\">WARNING: DirectML ONNX on Windows is not threadsafe. Using multithreaded PaddleOCR with DirectML will cause a crash with our current infra. "
            "Furthermore, with PaddleOCR, DirectML is slower than using the CPU. With other models, such as SAM, DirectML seems to be ~10% "
            "faster than the CPU. Cuda has not been tested.</font>",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , PRIORITY("<b>Thread Priority:</b>", DEFAULT_PRIORITY_NORMAL_INFERENCE)
        , MAX_INTRA_OP_THREADS(
            "<b>Maximum Intra-Op Threads:</b>",
            LockMode::UNLOCK_WHILE_RUNNING,
            m_default_max_threads,
            1
        )
        , MAX_INTER_OP_THREADS(
            "<b>Maximum Inter-Op Threads:</b><br>Zero means default behavior.",
            LockMode::UNLOCK_WHILE_RUNNING,
            0
        )
    {
        PA_ADD_OPTION(HARDWARE_THREADS);
        PA_ADD_STATIC(m_description);
        PA_ADD_OPTION(USE_GPU);
//        PA_ADD_OPTION(PRIORITY);  //  Not used yet.
        PA_ADD_OPTION(MAX_INTRA_OP_THREADS);
        PA_ADD_OPTION(MAX_INTER_OP_THREADS);

        HARDWARE_THREADS.set_visibility(ConfigOptionState::HIDDEN);
        USE_GPU.set_visibility(ConfigOptionState::HIDDEN);
    }

    virtual void load_json(const JsonValue& json) override{
        GroupOption::load_json(json);

        //  Reset the max threads if the hardware threads has changed.
        if (HARDWARE_THREADS != std::thread::hardware_concurrency()){
            MAX_INTRA_OP_THREADS.set(m_default_max_threads);
        }
    }

private:
    const int m_default_max_threads;
    SimpleIntegerOption<size_t> HARDWARE_THREADS;

public:
    StaticTextOption m_description;
    BooleanCheckBoxOption USE_GPU;
    ThreadPriorityOption PRIORITY;
    SimpleIntegerOption<int> MAX_INTRA_OP_THREADS;
    SimpleIntegerOption<int> MAX_INTER_OP_THREADS;
};





}
#endif
