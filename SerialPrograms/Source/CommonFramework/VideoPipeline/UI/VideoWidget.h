/*  Video Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoWidget_H
#define PokemonAutomation_VideoPipeline_VideoWidget_H

#include <QWidget>
#include "CommonFramework/VideoPipeline/CameraSession.h"

namespace PokemonAutomation{


//  Base class for the widget that displays the camera video feed.
class VideoWidget : public QWidget{
public:
    using QWidget::QWidget;
    virtual ~VideoWidget() = default;

    //  Get the camera associated with this VideoWidget.
    virtual CameraSession& camera() = 0;
};



}
#endif
