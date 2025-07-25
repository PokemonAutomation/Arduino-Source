/*  Video Source (Still Image)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSource_StillImage_H
#define PokemonAutomation_VideoPipeline_VideoSource_StillImage_H

#include <QImage>
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"

namespace PokemonAutomation{


class VideoSourceDescriptor_StillImage : public VideoSourceDescriptor{
public:
    VideoSourceDescriptor_StillImage()
        : VideoSourceDescriptor(VideoSourceType::StillImage)
    {}
    VideoSourceDescriptor_StillImage(std::string path)
        : VideoSourceDescriptor(VideoSourceType::StillImage)
        , m_path(std::move(path))
    {}

public:
    // get the image source path
    std::string path() const;
    // set the image source path
    void set_path(std::string path);

    virtual bool should_reload() const override{ return true; }
    virtual bool operator==(const VideoSourceDescriptor& x) const override;
    virtual std::string display_name() const override{
        return "Display Image";
    }

    virtual void run_post_select() override;
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<VideoSource> make_VideoSource(Logger& logger, Resolution resolution) const override;


private:
    mutable SpinLock m_lock;
    std::string m_path;
};



class VideoSource_StillImage : public VideoSource{
public:
    VideoSource_StillImage(Logger& logger, const std::string& path, Resolution resolution);

    const std::string path() const{
        return m_path;
    }

    virtual Resolution current_resolution() const override{
        return m_resolution;
    }
    virtual const std::vector<Resolution>& supported_resolutions() const override{
        return m_resolutions;
    }

    virtual VideoSnapshot snapshot_latest_blocking() override{
        return m_snapshot;
    }
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override{
        return m_snapshot;
    }

    virtual QWidget* make_display_QtWidget(QWidget* parent) override;

    const QImage& original_image() const { return m_original_image;}
private:
    friend class VideoWidget_StillImage;

    std::string m_path;
    QImage m_original_image;
    VideoSnapshot m_snapshot;
    Resolution m_resolution;
    std::vector<Resolution> m_resolutions;
};





}
#endif
