/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTypes/RGB32ImageView.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonLA_SelectedRegionDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MapLocationDetector : public VisualInferenceCallback{
public:
    MapLocationDetector(const QImage& screen)
        : VisualInferenceCallback("MapLocationDetector")
        , m_current_region(MapRegion::NONE)
    {
        m_regions.emplace_back(MapRegion::JUBILIFE,     ImageFloatBox(0.252, 0.400, 0.025, 0.150), screen);
        m_regions.emplace_back(MapRegion::FIELDLANDS,   ImageFloatBox(0.415, 0.550, 0.025, 0.150), screen);
        m_regions.emplace_back(MapRegion::MIRELANDS,    ImageFloatBox(0.750, 0.570, 0.025, 0.150), screen);
        m_regions.emplace_back(MapRegion::COASTLANDS,   ImageFloatBox(0.865, 0.240, 0.025, 0.150), screen);
        m_regions.emplace_back(MapRegion::HIGHLANDS,    ImageFloatBox(0.508, 0.320, 0.025, 0.150), screen);
        m_regions.emplace_back(MapRegion::ICELANDS,     ImageFloatBox(0.457, 0.060, 0.025, 0.150), screen);
        m_regions.emplace_back(MapRegion::RETREAT,      ImageFloatBox(0.635, 0.285, 0.025, 0.150), screen);
    }

    MapRegion current_region() const{
        return m_current_region;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override{
//        for (const RegionState& region : m_regions){
//            items.add(COLOR_CYAN, region.box);
//        }
    }

    virtual bool process_frame(const QImage& frame, WallClock timestamp) override{
        if (frame.isNull()){
            return false;
        }

        for (RegionState& region : m_regions){
            ImageViewRGB32 current = extract_box_reference(frame, region.box);

            if (current.width() != (size_t)region.start.width() || current.height() != (size_t)region.start.height()){
                region.start = current.to_QImage_owning();
                continue;
            }

            double rmsd = ImageMatch::pixel_RMSD(region.start, current);
            if (rmsd > 20){
                m_current_region = region.region;
                return true;
            }
        }
        return false;
    }

private:
    struct RegionState{
        MapRegion region;
        ImageFloatBox box;
        QImage start;
        RegionState(MapRegion p_region, const ImageFloatBox& p_box, const QImage& screen)
            : region(p_region)
            , box(p_box)
            , start(extract_box_copy(screen, box))
        {}
    };

    std::vector<RegionState> m_regions;
    MapRegion m_current_region;
};


MapRegion detect_selected_region(ConsoleHandle& console, BotBaseContext& context){
    MapLocationDetector detector(console.video().snapshot());
    int ret = wait_until(
        console, context,
        std::chrono::seconds(2),
        {{detector}}
    );
    MapRegion region = detector.current_region();
    if (ret < 0){
        console.log("Unable to detect active region on map.", COLOR_RED);
    }else{
        console.log(std::string("Current Selection: ") + MAP_REGION_NAMES[(int)region]);
    }
    return region;
}



}
}
}
