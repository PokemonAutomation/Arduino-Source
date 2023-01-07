/*  Screen Watch Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>
#include <QWidget>
#include <QPainter>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Qt/WidgetStackFixedAspectRatio.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "ScreenWatchOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




ScreenWatchOption::~ScreenWatchOption() = default;
ScreenWatchOption::ScreenWatchOption(
    std::string label,
    double default_x, double default_y,
    double default_width, double default_height
)
    : GroupOption(std::move(label), LockWhileRunning::UNLOCKED)
    , MONITOR_INDEX(
        "<b>Monitor Index:</b>",
        LockWhileRunning::UNLOCKED,
        0
    )
    , X("<b>X Coordinate:</b>", LockWhileRunning::UNLOCKED, default_x, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", LockWhileRunning::UNLOCKED, default_y, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", LockWhileRunning::UNLOCKED, default_width, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", LockWhileRunning::UNLOCKED, default_height, 0.0, 1.0)
    , m_display(CONSTRUCT_TOKEN, *this)
    , m_overlay(m_overlay_option)
{
    PA_ADD_OPTION(*m_display);
    PA_ADD_OPTION(MONITOR_INDEX);
    PA_ADD_OPTION(X);
    PA_ADD_OPTION(Y);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
}


double ScreenWatchOption::aspect_ratio(){
    qsizetype index = (qsizetype)MONITOR_INDEX;
    auto screens = QGuiApplication::screens();
    if (screens.size() <= index){
        return 0;
    }

    QScreen& screen = *screens[index];

    QSize size = screen.size();
    int width = (int)(size.width() * WIDTH + 0.5);
    int height = (int)(size.height() * HEIGHT + 0.5);
    return (double)width / height;
}
VideoSnapshot ScreenWatchOption::screenshot(){
    qsizetype index = (qsizetype)MONITOR_INDEX;
    auto screens = QGuiApplication::screens();
    if (screens.size() <= index){
        return VideoSnapshot();
    }

    QScreen& screen = *screens[index];

    QSize size = screen.size();
//    int width = screen.size().width();
//    int height = screen.size().height();

    int min_x = (int)(size.width() * X + 0.5);
    int min_y = (int)(size.height() * Y + 0.5);
    int width = (int)(size.width() * WIDTH + 0.5);
    int height = (int)(size.height() * HEIGHT + 0.5);
    WallClock now = current_time();
    QPixmap pm = screen.grabWindow(0, min_x, min_y, width, height);
    return VideoSnapshot(pm.toImage(), now);
}


VideoOverlaySession& ScreenWatchOption::overlay(){
    return m_overlay;
}





}
