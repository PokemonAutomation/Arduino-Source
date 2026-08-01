/*  Sandwich Hand Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Tests/TestUtils.h"
#include "PokemonSV_SandwichHandDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

class SandwichFreeHandMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichFreeHandMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/SandwichHand-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichFreeHandMatcher matcher;
        return matcher;
    }
};


class SandwichGrabbingHandMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichGrabbingHandMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/SandwichGrab-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichGrabbingHandMatcher matcher;
        return matcher;
    }
};

} // anonymous namespace

std::string SANDWICH_HAND_TYPE_NAMES(SandwichHandType type){
    switch (type)
    {
    case SandwichHandType::FREE:
        return "FREE";
    case SandwichHandType::GRABBING:
        return "GRABBING";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown SandwichHandType");
    }
}

SandwichHandLocator::SandwichHandLocator(HandType hand_type, const ImageFloatBox& box, Color color)
    : m_type(hand_type), m_box(box), m_color(color)
{}

void SandwichHandLocator::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

std::pair<double, double> SandwichHandLocator::detect(const ImageViewRGB32& frame) const{
    ImageFloatBox entire_screen(0.0, 0.0, 1.0, 1.0);
    std::pair<double, double> location = locate_sandwich_hand(frame, m_box);
    if (location.first >= 0.0){
        return location;
    }else{
        return locate_sandwich_hand(frame, entire_screen);
    }
    
}


std::pair<double, double> SandwichHandLocator::locate_sandwich_hand(const ImageViewRGB32& frame, ImageFloatBox area_to_search) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(150, 150, 150), combine_rgb(255, 255, 255)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);

    double min_hand_size = ((m_type == HandType::FREE) ? 5000.0 : 4500.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_hand_size);

    std::pair<double, double> hand_location(-1.0, -1.0);

    ImagePixelBox pixel_box = floatbox_to_pixelbox(frame.width(), frame.height(), area_to_search);
    match_template_by_waterfill(
        frame.size(),
        extract_box_reference(frame, area_to_search), 
        ((m_type == HandType::FREE) ? SandwichFreeHandMatcher::instance() : SandwichGrabbingHandMatcher::instance()),
        filters,
        {min_size, SIZE_MAX},
        80,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            hand_location = std::make_pair(
                (object.center_of_gravity_x() + pixel_box.min_x) / (double)frame.width(),
                (object.center_of_gravity_y() + pixel_box.min_y) / (double)frame.height()
            );
            return true;
        }
    );

    return hand_location;
}


SandwichHandWatcher::SandwichHandWatcher(
    HandType hand_type,
    const ImageFloatBox& box,
    Color color
)
    : VisualInferenceCallback("SandwichHandWatcher")
    , m_locator(hand_type, box, color)
    , m_location(-1.0, -1.0)
{}

void SandwichHandWatcher::make_overlays(VideoOverlaySet& items) const{
    m_locator.make_overlays(items);
}

bool SandwichHandWatcher::process_frame(const VideoSnapshot& frame){
    m_last_snapshot = frame;
    m_location = m_locator.detect(frame);
    return m_location.first >= 0.0;
}

bool SandwichHandWatcher::recover_sandwich_hand_position(const ImageViewRGB32& frame){
    ImageFloatBox entire_screen(0.0, 0.0, 1.0, 1.0);
    m_location = m_locator.locate_sandwich_hand(frame, entire_screen);
    return m_location.first >= 0.0;
}








class Test_SandwichHandWatcher : public UnitTest{
public:
    Test_SandwichHandWatcher(
        const std::string& image,
        std::vector<std::string> words
    )
        : UnitTest("PokemonSV::SandwichHandWatcher - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_words(std::move(words))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        // five words: hand_type("Free"/"Grabbing"), <image float box (four words total)>
        if (m_words.size() < 5){
            std::stringstream ss;
            ss << "Error: not enough number of words in the filename. Found only " << m_words.size() << "." << std::endl;
            return ss.str();
        }

        const auto& hand_type_word = m_words[m_words.size() - 5];
        SandwichHandLocator::HandType hand_type = SandwichHandLocator::HandType::FREE;
        if (hand_type_word == "Free"){
            hand_type = SandwichHandLocator::HandType::FREE;
        }else if (hand_type_word == "Grabbing"){
            hand_type = SandwichHandLocator::HandType::GRABBING;
        }else{
            std::stringstream ss;
            ss << "Error: word " << hand_type_word << " should be \"Free\" or \"Grabbing\"." << std::endl;
            return ss.str();
        }

        float box_values[4] = {0.0f};
        for (int i = 0; i < 4; i++){
            if (parse_float(m_words[m_words.size() - 4 + i], box_values[i]) == false){
                std::stringstream ss;
                ss << "Error: word " << m_words[m_words.size() - 4 + i] << " should be a float, range [0.0, 1.0]" << std::endl;
                return ss.str();
            }
        }

        ImageFloatBox box(box_values[0], box_values[1], box_values[2], box_values[3]);

        SandwichHandLocator detector(hand_type, box);

        bool hand_expected = true;
        if (m_words.size() >= 6){
            auto hand_expected_word = m_words[m_words.size() - 6];
            if(hand_expected_word == "False"){
                hand_expected = false;
            }
        }

        ImageRGB32 image(m_image);
        auto result = detector.detect(image);
        bool has_hand = result.first >= 0.0;

        TEST_RESULT_EQUAL_STR(has_hand, hand_expected);

        return true;
    };

private:
    std::string m_image;
    std::vector<std::string> m_words;
};




void add_tests_SandwichHandDetector(UnitTestDatabase& database){
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/2_False_Grabbing_0.0_0.0_1.0_1.0.png",
        std::vector<std::string>{"2", "False", "Grabbing", "0.0", "0.1", "1.0", "1.0"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/720p_Free_0.0_0.0_1.0_1.0.png",
        std::vector<std::string>{"720p", "Free", "0.0", "0.0", "1.0", "1.0"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/720p_Grabbing_0.0_0.0_1.0_1.0.png",
        std::vector<std::string>{"720p", "Grabbing", "0.0", "0.0", "1.0", "1.0"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/False_Grabbing_0.01_0.0_1.0_1.0.png",
        std::vector<std::string>{"False", "Grabbing", "0.01", "0.0", "1.0", "1.0"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/Free_0.376_0.176_0.261_0.372.png",
        std::vector<std::string>{"Free", "0.376", "0.176", "0.261", "0.372"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/macOS/Free_0.419_0.449_0.174_0.204.png",
        std::vector<std::string>{"Free", "0.419", "0.449", "0.174", "0.204"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/macOS/Free_0.421_0.024_0.163_0.174.png",
        std::vector<std::string>{"Free", "0.421", "0.024", "0.163", "0.174"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/macOS/Free_0.455_0.319_0.090_0.133.png",
        std::vector<std::string>{"Free", "0.455", "0.319", "0.090", "0.133"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/macOS/Grabbing_0.401_0.313_0.239_0.383.png",
        std::vector<std::string>{"Grabbing", "0.401", "0.313", "0.239", "0.383"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/macOS/Grabbing_0.448_0.295_0.110_0.164.png",
        std::vector<std::string>{"Grabbing", "0.448", "0.295", "0.110", "0.164"}
    );
    database.add<Test_SandwichHandWatcher>(
        "PokemonSV/SandwichHandDetector/macOS/Grabbing_0.454_0.037_0.099_0.160.png",
        std::vector<std::string>{"Grabbing", "0.454", "0.037", "0.099", "0.160"}
    );

}






}
}
}
