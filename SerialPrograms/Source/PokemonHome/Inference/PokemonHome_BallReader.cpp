/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonHome/Resources/PokemonHome_PokeballSprites.h"
#include "PokemonHome_BallReader.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{




PokeballSpriteMatcher::PokeballSpriteMatcher(double min_euclidean_distance)
    : CroppedImageDictionaryMatcher({1, 128})
    , m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
{
    for (const auto& item : PokemonHome::ALL_POKEBALL_SPRITES()){
        add(item.first, remove_white_border(item.second.sprite));
    }
}
ImageRGB32 PokeballSpriteMatcher::remove_white_border(const ImageViewRGB32& image){
    using namespace Kernels::Waterfill;

    ImageRGB32 ret = image.copy();
    {
        auto matrix = compress_rgb32_to_binary_range(ret, 0x00000000, 0x7f000000);
        auto session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(200);
        WaterfillObject object;
        if (!iter->find_next(object, true)){
            return ret;
        }
        filter_by_mask(object.packed_matrix(), ret, Color(0xffffffff), false);
    }
    {
        auto matrix = compress_rgb32_to_binary_range(ret, 0x00808080, 0xffffffff);
        auto session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(200);
        WaterfillObject object;
        if (!iter->find_next(object, true)){
            return ret;
        }
        filter_by_mask(object.packed_matrix(), ret, Color(0x00000000), false);
    }
    return ret;
}
auto PokeballSpriteMatcher::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
    ImageStats border = image_border_stats(image);
    ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        image,
        [&](Color pixel){
            double r = (double)pixel.red() - border.average.r;
            double g = (double)pixel.green() - border.average.g;
            double b = (double)pixel.blue() - border.average.b;
            bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
            return stop;
        }
    );
    std::vector<ImageViewRGB32> ret;
    ret.emplace_back(extract_box_reference(image, box));
    return ret;
}






const double BallReader::MAX_ALPHA = 0.40;
const double BallReader::ALPHA_SPREAD = 0.02;


const PokeballSpriteMatcher& BALL_SPRITE_MATCHER(){
    static PokeballSpriteMatcher matcher;
    return matcher;
}



BallReader::BallReader(VideoStream& stream)
    : m_matcher(BALL_SPRITE_MATCHER())
    , m_stream(stream)
    , m_box_sprite(stream.overlay(), {0.228, 0.095, 0.030, 0.049})
{}



std::string BallReader::read_ball(const ImageViewRGB32& screen) const{
    if (!screen){
        return "";
    }

    ImageMatch::ImageMatchResult sprite_result;
    {
        ImageViewRGB32 image = extract_box_reference(screen, m_box_sprite);
        sprite_result = m_matcher.match(image, ALPHA_SPREAD);
        sprite_result.log(m_stream.logger(), 0.50);
        if (!sprite_result.results.empty() && sprite_result.results.begin()->first > MAX_ALPHA){
            sprite_result.results.clear();
        }
    }

    if (sprite_result.results.size() != 1){
        dump_image(m_stream.logger(), ProgramInfo(), "BallReader", screen);
    }
    if (sprite_result.results.empty()){
        return "";
    }

    return sprite_result.results.begin()->second;
}

class Test_BallReader : public UnitTest{
public:

    Test_BallReader(
        const std::string& image,
        const std::string& expected
    )
        : UnitTest("PokemonHome::BallReader - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyAudioFeed audio_feed;
        DummyVideoFeed video_feed;
        StreamHistorySession history(logger);
        DummyVideoOverlay video_overlay;
        VideoStream stream(logger, audio_feed, video_feed, history, video_overlay);
        ImageRGB32 image(m_image);
        BallReader reader(stream);
        std::string result = reader.read_ball(image);

        if (result == m_expected)
            return true;

        return "Expected: " + m_expected + ", received: " + result;
    };

private:
    std::string m_image;
    std::string m_expected;
};

void add_tests_BallReader(UnitTestDatabase& database){
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/annihilape_Regular.png", "poke-ball");
}



}
}
}
