/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLA_SelectedRegionDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MapLocationDetector : public VisualInferenceCallback{
public:
    MapLocationDetector()
        : VisualInferenceCallback("MapLocationDetector")
        , m_current_region(MapRegion::NONE)
    {}

    MapRegion current_region() const{
        return m_current_region;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override{
//        for (const RegionState& region : m_regions){
//            items.add(COLOR_CYAN, region.box);
//        }
    }

    virtual bool process_frame(const VideoSnapshot& frame) override{
        if (!frame){
            return false;
        }
        if (m_regions.empty()){
            reload_reference(frame.frame);
        }

        MapRegion current_region = MapRegion::NONE;
        for (RegionState& region : m_regions){
            ImageViewRGB32 current = extract_box_reference(frame, region.box);

            if (current.width() != (size_t)region.start.width() || current.height() != (size_t)region.start.height()){
                reload_reference(frame.frame);
                return false;
            }

            double rmsd = ImageMatch::pixel_RMSD(region.start, current);
            if (rmsd > 20){
                if (current_region == MapRegion::NONE){
                    current_region = region.region;
                }else{
                    //  Multiple large diffs. Frame isn't stable.
//                    cout << "Multiple diffs: " << rmsd << endl;
                    reload_reference(frame.frame);
                    return false;
                }
            }
        }

        m_current_region = current_region;
        return current_region != MapRegion::NONE;
    }

    void reload_reference(std::shared_ptr<const ImageRGB32> screen){
        m_screen = std::move(screen);
        m_regions.clear();
        m_regions.emplace_back(MapRegion::JUBILIFE,     ImageFloatBox(0.252, 0.400, 0.025, 0.150), *m_screen);
        m_regions.emplace_back(MapRegion::FIELDLANDS,   ImageFloatBox(0.415, 0.550, 0.025, 0.150), *m_screen);
        m_regions.emplace_back(MapRegion::MIRELANDS,    ImageFloatBox(0.750, 0.570, 0.025, 0.150), *m_screen);
        m_regions.emplace_back(MapRegion::COASTLANDS,   ImageFloatBox(0.865, 0.240, 0.025, 0.150), *m_screen);
        m_regions.emplace_back(MapRegion::HIGHLANDS,    ImageFloatBox(0.508, 0.320, 0.025, 0.150), *m_screen);
        m_regions.emplace_back(MapRegion::ICELANDS,     ImageFloatBox(0.457, 0.060, 0.025, 0.150), *m_screen);
        m_regions.emplace_back(MapRegion::RETREAT,      ImageFloatBox(0.635, 0.285, 0.025, 0.150), *m_screen);
    }

private:
    struct RegionState{
        MapRegion region;
        ImageFloatBox box;
        ImageViewRGB32 start;
        RegionState(MapRegion p_region, const ImageFloatBox& p_box, const ImageViewRGB32& screen)
            : region(p_region)
            , box(p_box)
            , start(extract_box_reference(screen, box))
        {}
    };

    std::shared_ptr<const ImageRGB32> m_screen;
    std::vector<RegionState> m_regions;
    MapRegion m_current_region;
};


MapRegion detect_selected_region(VideoStream& stream, CancellableScope& scope){
    MapLocationDetector detector;
    int ret = wait_until(
        stream, scope,
        std::chrono::seconds(2),
        {{detector}}
    );
    MapRegion region = detector.current_region();
    if (ret < 0){
        stream.log("Unable to detect active region on map.", COLOR_RED);
    }else{
        stream.log(std::string("Current Selection: ") + MAP_REGION_NAMES[(int)region]);
    }
    return region;
}



}
}
}
