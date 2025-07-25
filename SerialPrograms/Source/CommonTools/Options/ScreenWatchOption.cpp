/*  Screen Watch Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>
#include <QWidget>
#include <QPainter>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/Logging/Logger.h"
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
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , MONITOR_INDEX(
        "<b>Monitor Index:</b> For multi-monitor setups, this lets you choose which monitor to watch.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , X(
        "<b>X Coordinate:</b> The left edge of the box to watch.<br>"
        "0.0 is the left edge of the monitor. 1.0 is the right edge of the monitor.",
        LockMode::UNLOCK_WHILE_RUNNING,
        default_x, 0.0, 1.0
    )
    , Y(
        "<b>Y Coordinate:</b> The top edge of the box to watch.<br>"
        "0.0 is the top edge of the monitor. 1.0 is the bottom edge of the monitor.",
        LockMode::UNLOCK_WHILE_RUNNING,
        default_y, 0.0, 1.0
    )
    , WIDTH(
        "<b>Width:</b> The width of the box to watch.<br>"
        "The number is between 0 and 1 and is the proportion of the full width of the monitor.",
        LockMode::UNLOCK_WHILE_RUNNING,
        default_width, 0.0, 1.0
    )
    , HEIGHT(
        "<b>Height:</b> The height of the box to watch.<br>"
        "The number is between 0 and 1 and is the proportion of the full height of the monitor.",
        LockMode::UNLOCK_WHILE_RUNNING,
        default_height, 0.0, 1.0
    )
    , m_display(*this)
    , m_buttons(*this)
    , m_overlay(global_logger_tagged(), m_overlay_option)
{
    PA_ADD_OPTION(m_display);
    PA_ADD_OPTION(MONITOR_INDEX);
    PA_ADD_OPTION(X);
    PA_ADD_OPTION(Y);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
    PA_ADD_OPTION(m_buttons);
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
