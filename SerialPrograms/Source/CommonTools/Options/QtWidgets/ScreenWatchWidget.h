/*  Screen Watch Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_ScreenWatchWidget_H
#define PokemonAutomation_CommonTools_Options_ScreenWatchWidget_H

#include <QWidget>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Qt/WidgetStackFixedAspectRatio.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/UI/VideoOverlayWidget.h"
#include "CommonTools/Options/ScreenWatchOption.h"

namespace PokemonAutomation{

class ScreenWatchWidget;


class ScreenWatchDisplayWidget : public QWidget{
public:
    ScreenWatchDisplayWidget(ScreenWatchWidget& parent, ScreenWatchOption& option);
    ~ScreenWatchDisplayWidget();
    void paintEvent(QPaintEvent* event) override;

private:
    void thread_loop();

private:
    ScreenWatchWidget& m_holder;
    ScreenWatchOption& m_option;
    Mutex m_lock;
    ConditionVariable m_cv;
    bool m_stop;
    VideoSnapshot m_last_frame;
    std::unique_ptr<AsyncTask> m_updater;
};

class ScreenWatchWidget : public WidgetStackFixedAspectRatio, public ConfigWidget{
public:
    using ParentOption = ScreenWatchDisplay;

public:
    ScreenWatchWidget(QWidget& parent, ScreenWatchDisplay& option);

private:
    QWidget* m_widget;
    VideoOverlayWidget* m_overlay;
};

class ScreenWatchButtonWidget : public QWidget, public ConfigWidget{
public:
    using ParentOption = ScreenWatchButtons;

public:
    ScreenWatchButtonWidget(QWidget& parent, ScreenWatchButtons& option);
};


}
#endif
