/*  Video Overlay Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoOverlayWidget_H
#define PokemonAutomation_VideoPipeline_VideoOverlayWidget_H

#include <map>
#include <QWidget>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"

namespace PokemonAutomation{

struct OverlayText;

class VideoOverlayWidget : public QWidget, private VideoOverlaySession::Listener{
public:
    ~VideoOverlayWidget();
    VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session);

    void update_size(const QSize& widget_size, const QSize& video_size);

    // Whether to hide inference visualization
    void set_inference_hidden(bool hidden) { m_inference_hidden = hidden;}
    // Whether to hide log text visualization
    void set_log_text_hidden(bool hidden) {m_log_hidden = hidden;}

private:
    // Override VideoOverlaySession::Listener::box_update().
    // This function is called asynchronously when there is change of boxes in VideoOverlaySession.
    virtual void box_update(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& boxes) override;
    // Override VideoOverlaySession::Listener::text_update().
    // This function is called asynchronously when there is change of texts in VideoOverlaySession.
    virtual void text_update(const std::shared_ptr<const std::vector<OverlayText>>& texts) override;
    // Override VideoOverlaySession::Listener::log_text_update().
    // This function is called asynchronously when there is change of log texts in VideoOverlaySession.
    virtual void log_text_update(const std::shared_ptr<const std::vector<OverlayText>>& texts) override;
    // Override VideoOverlaySession::Listener::log_text_background_update().
    // This function is called asynchronously when there is change of log text background in VideoOverlaySession.
    virtual void log_text_background_update(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& bg_boxes) override;

    virtual void paintEvent(QPaintEvent*) override;

private:
    VideoOverlaySession& m_session;

    QSize m_video_size;
    QSize m_display_size;
    int m_offset_x;
    double m_scale;

    SpinLock m_lock;
    std::shared_ptr<const std::vector<VideoOverlaySession::Box>> m_boxes;
    std::shared_ptr<const std::vector<OverlayText>> m_texts;
    std::shared_ptr<const std::vector<OverlayText>> m_log_texts;
    std::shared_ptr<const std::vector<VideoOverlaySession::Box>> m_log_text_bg_boxes;

    bool m_inference_hidden;
    bool m_log_hidden;
};


}
#endif

