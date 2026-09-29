/*  Format Aggregator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "VideoFrameQt.h"
#include "QFormatAggregator.h"

namespace PokemonAutomation{


void get_format(
    const QCameraFormat& qformat,
    Resolution& resolution,
    VideoFormat& format,
    FramesPerSecond& fps
){
    resolution.width = qformat.resolution().width();
    resolution.height = qformat.resolution().height();
    format = QVideoFrameFormat_to_VideoFormat(qformat.pixelFormat());
    float fps_f = qformat.maxFrameRate();
    fps_f = std::max<float>(fps_f, 0);
    fps = (size_t)(fps_f + 0.5);
}


struct FormatAggregator{
    std::string m_format_dump_str;
    std::map<
        Resolution,
        std::map<
            VideoFormat,
            std::map<size_t, QCameraFormat, std::greater<size_t>>
        >
    > m_formats;

    void add_format(QCameraFormat qformat){
        Resolution resolution;
        VideoFormat oformat;
        FramesPerSecond fps;
        get_format(qformat, resolution, oformat, fps);

        m_format_dump_str += "\n    ";
        m_format_dump_str += std::to_string(resolution.width) + "x" + std::to_string(resolution.height);
        m_format_dump_str += ": Format = " + std::to_string((int)qformat.pixelFormat());
        m_format_dump_str += ", FPS Range = [" + std::to_string(qformat.minFrameRate()) + "," + std::to_string(qformat.maxFrameRate()) + "]";

        auto& resolution_entry = m_formats[resolution];
        auto iter0 = resolution_entry.find(oformat);

        //  Format doesn't exist. Add it.
        if (iter0 == resolution_entry.end()){
            auto& entry = resolution_entry[oformat];
            entry[fps] = std::move(qformat);
            return;
        }

        //  Format already exists.

        auto& oformat_node = iter0->second;

        auto iter1 = oformat_node.find(fps);
        if (iter1 == oformat_node.end()){
            oformat_node[fps] = std::move(qformat);
            return;
        }

        if (iter1->second.minFrameRate() < qformat.minFrameRate()){
            iter1->second = std::move(qformat);
        }
    }
};



QCameraFormat build_format_set(
    Logger& logger,
    VideoFormatSet& format_set,
    const QCameraDevice& device,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
){
    QList<QCameraFormat> formats = device.videoFormats();
    if (formats.empty()){
        logger.log("No usable resolutions: " + device.description().toStdString(), COLOR_RED);
        return QCameraFormat();
    }

    FormatAggregator aggregator;
    for (QCameraFormat& format : formats){
        aggregator.add_format(std::move(format));
    }

    if (GlobalSettings::instance().DUMP_VIDEO_FORMATS){
        logger.log("Video Formats:" + aggregator.m_format_dump_str);
    }

//    cout << "Chosen: " << resolution_map[Resolution(3840, 2160)]->maxFrameRate() << endl;

    //  Set a default.
    const QCameraFormat* current_qformat = nullptr;
    const std::vector<Resolution> RESOLUTION_PRIORITIES{
        {3840, 2160},
        {1920, 1080},
        {2560, 1440},
        {1280, 720},
    };
    for (const Resolution& res : RESOLUTION_PRIORITIES){
        auto iter = aggregator.m_formats.find(res);
        if (iter != aggregator.m_formats.end()){
            current_qformat = &iter->second.begin()->second.begin()->second;
            break;
        }
    }
    if (current_qformat == nullptr){
        current_qformat = &aggregator.m_formats.rbegin()->second.begin()->second.begin()->second;
    }

    format_set.clear();
    for (const auto& res : aggregator.m_formats){
        //  Resolution matches. Pick the first one as the default.
        if (res.first == desired_resolution){
            current_qformat = &res.second.begin()->second.begin()->second;
        }

        for (const auto& format : res.second){
            //  Format matches. Pick the first one as the default.
            if (res.first == desired_resolution && format.first == desired_format){
                current_qformat = &format.second.begin()->second;
            }

            for (const auto& fps : format.second){
                //  FPS matches. Pick it.
                if (res.first == desired_resolution &&
                    format.first == desired_format &&
                    fps.first == desired_fps
                ){
                    current_qformat = &fps.second;
                }

                format_set[res.first][format.first].insert(fps.first);
            }
        }
    }

    return *current_qformat;
}



}
