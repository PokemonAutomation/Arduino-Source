/*  Video Overlay Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoOverlayWidget_H
#define PokemonAutomation_VideoPipeline_VideoOverlayWidget_H

#include <QWidget>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"

namespace PokemonAutomation{

struct OverlayText;

class VideoOverlayWidget : public QWidget, private VideoOverlaySession::ContentListener, private WatchdogCallback{
public:
    static constexpr bool DEFAULT_ENABLE_BOXES  = true;
    static constexpr bool DEFAULT_ENABLE_TEXT   = true;
    static constexpr bool DEFAULT_ENABLE_IMAGES = true;
    static constexpr bool DEFAULT_ENABLE_LOG    = false;
    static constexpr bool DEFAULT_ENABLE_STATS  = true;

public:
    ~VideoOverlayWidget();
    VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session);

private:
    void detach();

    //  Asynchronous changes to the overlays.

    // callback function from VideoOverlaySession on overlay boxes enabled
    virtual void on_overlay_enabled_boxes (bool enabled) override{async_update();}
    // callback function from VideoOverlaySession on overlay text enabled
    virtual void on_overlay_enabled_text  (bool enabled) override{async_update();}
    // callback function from VideoOverlaySession on overlay images enabled
    virtual void on_overlay_enabled_images(bool enabled) override{async_update();}
    // callback function from VideoOverlaySession on overlay log enabled
    virtual void on_overlay_enabled_log   (bool enabled) override{async_update();}
    // callback function from VideoOverlaySession on overlay stats enabled
    virtual void on_overlay_enabled_stats (bool enabled) override{async_update();}

    // callback function from VideoOverlaySession on overlay boxes updated
    virtual void on_overlay_update_boxes (const std::shared_ptr<const std::vector<OverlayBox>>& boxes) override;
    // callback function from VideoOverlaySession on overlay text updated
    virtual void on_overlay_update_text  (const std::shared_ptr<const std::vector<OverlayText>>& texts) override;
    // callback function from VideoOverlaySession on overlay images updated
    virtual void on_overlay_update_images(const std::shared_ptr<const std::vector<OverlayImage>>& images) override;
    // callback function from VideoOverlaySession on overlay images updated
    virtual void on_overlay_update_log   (const std::shared_ptr<const std::vector<OverlayLogLine>>& logs) override;

    virtual void on_watchdog_timeout() override;

    virtual void resizeEvent(QResizeEvent* event) override;
    // render video overlay, override QWidget::paintEvent()
    virtual void paintEvent(QPaintEvent*) override;

private:
    // Call QWidget::update() to notify Qt to schedule a re-rendering of the overlay widget.
    //
    // Threads other than the main thread may change video overlay, causing this VideoOverlayWidget, which
    // listens to video overlay change, to get called on the non-main thread.
    // Since QWidget::update() must be called on the main thread to not crash, we have to use this
    // async_update() to avoid that.
    void async_update();

    // render overlay boxes
    void render_boxes  (QPainter& painter);
    // render overlay texts
    void render_text   (QPainter& painter);
    // render overlay images
    void render_images (QPainter& painter);
    // render overlay log lines
    void render_log    (QPainter& painter);
    // render overlay stats
    void render_stats  (QPainter& painter);

private:
    VideoOverlaySession& m_session;

    SpinLock m_lock;
    std::shared_ptr<const std::vector<OverlayBox>> m_boxes;
    std::shared_ptr<const std::vector<OverlayText>> m_texts;
    std::shared_ptr<const std::vector<OverlayImage>> m_images;
    std::shared_ptr<const std::vector<OverlayLogLine>> m_log;

    PeriodicStatsReporterI32 m_stats_paint;
};


}
#endif

