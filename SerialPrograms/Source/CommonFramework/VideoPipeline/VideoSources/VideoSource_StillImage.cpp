/*  Video Source (Still Image)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QWidget>
#include <QPainter>
#include <QFileDialog>
#include "VideoSource_StillImage.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



bool VideoSourceDescriptor_StillImage::operator==(const VideoSourceDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }

    std::string other_path = static_cast<const VideoSourceDescriptor_StillImage&>(x).m_path;

    ReadSpinLock lg(m_lock);
    return m_path == other_path;
}

std::string VideoSourceDescriptor_StillImage::path() const{
    ReadSpinLock lg(m_lock);
    return m_path;
}
void VideoSourceDescriptor_StillImage::set_path(std::string path){
    WriteSpinLock lg(m_lock);
    m_path = std::move(path);
}

void VideoSourceDescriptor_StillImage::run_post_select(){
//    if (!m_path.empty()){
//        return;
//    }
    std::string path = QFileDialog::getOpenFileName(
        nullptr, "Open image file", ".", "*.png *.jpg"
    ).toStdString();
    set_path(std::move(path));
}
void VideoSourceDescriptor_StillImage::load_json(const JsonValue& json){
//    cout << "load_json: " << m_path << endl;
    const std::string* name = json.to_string();
    if (name != nullptr){
        WriteSpinLock lg(m_lock);
        m_path = *name;
    }
}
JsonValue VideoSourceDescriptor_StillImage::to_json() const{
    ReadSpinLock lg(m_lock);
    return m_path;
}

std::unique_ptr<VideoSource> VideoSourceDescriptor_StillImage::make_VideoSource(Logger& logger, Resolution resolution) const{
//    cout << "make_VideoSource: " << m_path << endl;
    return std::make_unique<VideoSource_StillImage>(logger, path(), resolution);
}





VideoSource_StillImage::VideoSource_StillImage(Logger& logger, const std::string& path, Resolution resolution)
    : VideoSource(logger, false)
    , m_original_image(QString::fromStdString(path))
{
    m_snapshot = VideoSnapshot(
        ImageRGB32(m_original_image).scale_to(resolution.width, resolution.height),
        current_time()
    );
    m_resolution = resolution;
    m_resolutions = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160},
        {(size_t)m_original_image.width(), (size_t)m_original_image.height()}
    };
}



class VideoWidget_StillImage : public QWidget{
public:
    VideoWidget_StillImage(QWidget* parent, VideoSource_StillImage& source)
        : QWidget(parent)
        , m_source(source)
    {}

private:
    virtual void paintEvent(QPaintEvent* event) override{
        QWidget::paintEvent(event);

        QRect rect(0, 0, this->width(), this->height());
        QPainter painter(this);
        painter.drawImage(rect, m_source.m_original_image);
    }

private:
    VideoSource_StillImage& m_source;
};





QWidget* VideoSource_StillImage::make_display_QtWidget(QWidget* parent){
    return new VideoWidget_StillImage(parent, *this);
}




}

