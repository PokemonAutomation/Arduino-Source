/*  Video Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoWidget_H
#define PokemonAutomation_VideoWidget_H

#include <string>
#include <vector>
#include <QImage>
#include <QWidget>

namespace PokemonAutomation{


class CameraInfo{
public:
    CameraInfo() = default;
    explicit CameraInfo(std::string device_name)
        : m_device_name(std::move(device_name))
    {}

    operator bool() const{ return !m_device_name.empty(); }
    const std::string& device_name() const{ return m_device_name; }

    bool operator==(const CameraInfo& info){
        return m_device_name == info.m_device_name;
    }

private:
    std::string m_device_name;
};


class VideoWidget : public QWidget{
public:
    using QWidget::QWidget;
    virtual ~VideoWidget() = default;

    //  These will be called from the UI thread.
    virtual QSize resolution() const = 0;
    virtual std::vector<QSize> resolutions() const = 0;
    virtual void set_resolution(const QSize& size) = 0;

    //  This snapshot function will be called asynchronously from any thread
    //  except the UI thread. It needs to be thread safe with itself and the
    //  above functions.
    virtual QImage snapshot() = 0;
};



}
#endif
