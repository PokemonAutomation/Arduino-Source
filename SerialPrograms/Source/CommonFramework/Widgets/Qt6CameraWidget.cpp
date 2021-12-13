/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QVBoxLayout>
#include "Qt6CameraWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


std::vector<CameraInfo> qt6_get_all_cameras(){
    std::vector<CameraInfo> ret;
    return ret;
}

QString qt6_get_camera_name(const CameraInfo& info){
    return "";
}

Qt6VideoWidget::Qt6VideoWidget(
    QWidget* parent,
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : VideoWidget(parent)
    , m_logger(logger)
{
    if (!info){
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);

    //  TODO
}
Qt6VideoWidget::~Qt6VideoWidget(){

}

QSize Qt6VideoWidget::resolution() const{
    return QSize();
}
std::vector<QSize> Qt6VideoWidget::resolutions() const{
    return m_resolutions;
}
void Qt6VideoWidget::set_resolution(const QSize& size){

}
QImage Qt6VideoWidget::snapshot(){
    return QImage();
}



}
#endif
