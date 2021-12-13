/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Qt6VideoWidget_H
#define PokemonAutomation_Qt6VideoWidget_H

#include "CommonFramework/Tools/Logger.h"
#include "VideoWidget.h"

namespace PokemonAutomation{


std::vector<CameraInfo> qt6_get_all_cameras();
QString qt6_get_camera_name(const CameraInfo& info);


class Qt6VideoWidget : public VideoWidget{
public:
    Qt6VideoWidget(
        QWidget* parent,
        Logger& logger,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~Qt6VideoWidget();
    virtual QSize resolution() const override;
    virtual std::vector<QSize> resolutions() const override;
    virtual void set_resolution(const QSize& size) override;

    //  Cannot call from UI thread or it will deadlock.
    virtual QImage snapshot() override;


private:
    Logger& m_logger;
    std::vector<QSize> m_resolutions;

    //  TODO

};


}
#endif
