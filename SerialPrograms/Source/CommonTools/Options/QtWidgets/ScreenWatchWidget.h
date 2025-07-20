/*  Screen Watch Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_ScreenWatchWidget_H
#define PokemonAutomation_CommonTools_Options_ScreenWatchWidget_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <QWidget>
#include "Common/Qt/WidgetStackFixedAspectRatio.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/UI/VideoOverlayWidget.h"
#include "CommonTools/Options/ScreenWatchOption.h"

namespace PokemonAutomation{

class ScreenWatchWidget;


class ScreenWatchDisplayWidget : public QWidget{
public:
    ScreenWatchDisplayWidget(ScreenWatchOption& option, ScreenWatchWidget& parent);
    ~ScreenWatchDisplayWidget();
    void paintEvent(QPaintEvent* event) override;

private:
    void thread_loop();

private:
    ScreenWatchWidget& m_holder;
    ScreenWatchOption& m_option;
    std::mutex m_lock;
    std::condition_variable m_cv;
    bool m_stop;
    VideoSnapshot m_last_frame;
    std::thread m_updater;
};

class ScreenWatchWidget : public WidgetStackFixedAspectRatio, public ConfigWidget{
public:
    ScreenWatchWidget(ScreenWatchDisplay& option, QWidget& parent);

private:
    QWidget* m_widget;
    VideoOverlayWidget* m_overlay;
};

class ScreenWatchButtonWidget : public QWidget, public ConfigWidget{
public:
    ScreenWatchButtonWidget(ScreenWatchOption& option, QWidget& parent);
};


}
#endif
