/*  Video Overlay Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoOverlayWidget_H
#define PokemonAutomation_VideoPipeline_VideoOverlayWidget_H

#include <map>
#include <QWidget>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"

namespace PokemonAutomation{

struct OverlayText;

class VideoOverlayWidget : public QWidget, private VideoOverlaySession::Listener, private WatchdogCallback{
public:
    static constexpr bool DEFAULT_ENABLE_BOXES  = true;
    static constexpr bool DEFAULT_ENABLE_TEXT   = true;
    static constexpr bool DEFAULT_ENABLE_LOG    = false;
    static constexpr bool DEFAULT_ENABLE_STATS  = true;

public:
    ~VideoOverlayWidget();
    VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session);

private:
    void detach();

    //  Asynchronous changes to the overlays.

    virtual void enabled_boxes(bool enabled) override;
    virtual void enabled_text (bool enabled) override;
    virtual void enabled_log  (bool enabled) override;
    virtual void enabled_stats(bool enabled) override;

    virtual void update_boxes(const std::shared_ptr<const std::vector<OverlayBox>>& boxes) override;
    virtual void update_text (const std::shared_ptr<const std::vector<OverlayText>>& texts) override;
    virtual void update_log  (const std::shared_ptr<const std::vector<OverlayLogLine>>& texts) override;
    virtual void update_stats(const std::list<OverlayStat*>* stats) override;

    virtual void on_watchdog_timeout() override;

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
    void update_boxes(QPainter& painter);
    void update_text (QPainter& painter);
    void update_log  (QPainter& painter);
    void update_stats(QPainter& painter);

private:
    VideoOverlaySession& m_session;

    SpinLock m_lock;
    std::shared_ptr<const std::vector<OverlayBox>> m_boxes;
    std::shared_ptr<const std::vector<OverlayText>> m_texts;
    std::shared_ptr<const std::vector<OverlayLogLine>> m_log;
    const std::list<OverlayStat*>* m_stats;
};


}
#endif

