/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceCallback_H
#define PokemonAutomation_CommonFramework_VisualInferenceCallback_H

#include <QImage>

namespace PokemonAutomation{

class InterruptableCommandSession;


class VisualInferenceCallback{
public:
    //  Return true if the inference session should stop.
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) = 0;
};



class VisualInferenceCallbackWithCommandStop : public VisualInferenceCallback{
public:
    VisualInferenceCallbackWithCommandStop();

    void register_command_stop(InterruptableCommandSession& session);
    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) = 0;

    //  Returns true if this callback has returned true at least once.
    bool triggered(){
        return m_triggered.load(std::memory_order_acquire);
    }

private:
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    std::atomic<bool> m_triggered;
    std::vector<InterruptableCommandSession*> m_command_stops;
};




}
#endif
