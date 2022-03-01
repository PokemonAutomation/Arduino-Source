/*  Selected Region Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_SelectedRegionDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const char* MAP_REGION_NAMES[] = {
    "None",
    "Jubilife Village",
    "Obsidian Fieldlands",
    "Crimson Mirelands",
    "Cobalt Coastlands",
    "Coronet Highlands",
    "Alabaster Islands",
    "Ancient Retreat",
};


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

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override{
        if (frame.isNull()){
            return false;
        }

        for (RegionState& region : m_regions){
            QImage current = extract_box(frame, region.box);

            if (current.size() != region.start.size()){
                region.start = current;
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
            , start(extract_box(screen, box))
        {}
    };

    std::vector<RegionState> m_regions;
    MapRegion m_current_region;
};


MapRegion detect_selected_region(ProgramEnvironment& env, ConsoleHandle& console){
    MapLocationDetector detector(console.video().snapshot());
    int ret = wait_until(
        env, console,
        std::chrono::seconds(2),
        { &detector }
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
