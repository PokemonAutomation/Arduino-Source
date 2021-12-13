/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceCallback_H
#define PokemonAutomation_CommonFramework_VisualInferenceCallback_H

#include <vector>
#include <deque>
#include <QImage>
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{

class InterruptableCommandSession;


class ScreenDetector{
public:
    virtual bool detect(const QImage& screen) const = 0;
};


class VisualInferenceCallback{
public:
    //  Return true if the inference session should stop.
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) = 0;

public:
    void make_overlays(std::deque<InferenceBoxScope>& boxes, VideoOverlay& overlay){
        for (const auto& item : m_boxes){
            boxes.emplace_back(overlay, *item.first, item.second);
        }
    }

protected:
    void add_box(const ImageFloatBox& box, QColor color = Qt::red){
        m_boxes.emplace_back(&box, color);
    }
    void add_boxes(const VisualInferenceCallback& callback){
        for (const auto& item : callback.m_boxes){
            m_boxes.emplace_back(item);
        }
    }

private:
    std::vector<std::pair<const ImageFloatBox*, QColor>> m_boxes;
};



#if 0
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
#endif






}
#endif
