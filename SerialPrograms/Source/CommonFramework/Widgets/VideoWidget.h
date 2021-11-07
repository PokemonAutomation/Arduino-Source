/*  Video Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoWidget_H
#define PokemonAutomation_VideoWidget_H

#include <vector>
#include <QImage>
#include <QWidget>

namespace PokemonAutomation{


class VideoWidget : public QWidget{
public:
    VideoWidget(QWidget& parent)
        : QWidget(&parent)
    {}
    virtual ~VideoWidget() = default;

    //  These will be called from the UI thread.
    virtual QSize resolution() = 0;
    virtual std::vector<QSize> resolutions() = 0;
    virtual void set_resolution(const QSize& size) = 0;

    //  This snapshot function will be called asynchronously from any thread
    //  except the UI thread. It needs to be thread safe with itself and the
    //  above functions.
    virtual QImage snapshot() = 0;
};



}
#endif
