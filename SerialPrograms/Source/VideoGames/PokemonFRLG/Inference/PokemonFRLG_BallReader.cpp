/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/Resources/SpriteDatabase.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace{

constexpr double MAX_ALPHA = 0.40;
constexpr double ALPHA_SPREAD = 0.02;
constexpr double MIN_BACKGROUND_DISTANCE = 100;
const ImageFloatBox BALL_BOX(0.410, 0.507, 0.063, 0.084);

class PokeballSpriteMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    PokeballSpriteMatcher()
        : CroppedImageDictionaryMatcher({1, 128})
    {
        const SpriteDatabase sprites("PokemonFRLG/PokeballSprites.png", "PokemonFRLG/PokeballSprites.json");
        for (const auto& item : sprites){
            add(item.first, item.second.sprite);
        }
    }

private:
    std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override{
        const FloatPixel background = image_border_stats(image).average;
        const ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [&](Color pixel){
                double r = (double)pixel.red() - background.r;
                double g = (double)pixel.green() - background.g;
                double b = (double)pixel.blue() - background.b;
                return r*r + g*g + b*b >= MIN_BACKGROUND_DISTANCE * MIN_BACKGROUND_DISTANCE;
            }
        );
        return {extract_box_reference(image, box)};
    }
};

}


BallReader::BallReader(VideoStream& stream)
    : m_stream(stream)
    , m_box_sprite(stream.overlay(), GameSettings::instance().GAME_BOX.inner_to_outer(BALL_BOX))
{}

std::string BallReader::read_ball(const ImageViewRGB32& screen) const{
    if (!screen){
        return "";
    }

    static const PokeballSpriteMatcher matcher;
    const ImageViewRGB32 image = extract_box_reference(screen, m_box_sprite);
    ImageMatch::ImageMatchResult result = matcher.match(image, ALPHA_SPREAD);
    result.log(m_stream.logger(), MAX_ALPHA);
    result.clear_beyond_alpha(MAX_ALPHA);

    if (result.results.size() != 1){
        dump_image(m_stream.logger(), ProgramInfo(), "FRLG-BallReader", screen);
    }
    if (result.results.empty()){
        return "";
    }
    return result.results.begin()->second;
}


class Test_BallReader : public ImageUnitTest{
public:
    Test_BallReader(const std::string& image, std::string expected)
        : ImageUnitTest("PokemonFRLG::BallReader - " + image, {"PokemonFRLG/StatsReader/Page1/" + image})
        , m_expected(std::move(expected))
    {}

private:
    UnitTestResult run_on_images(
        Logger& logger, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        DummyAudioFeed audio_feed;
        DummyVideoFeed video_feed;
        StreamHistorySession history(logger);
        DummyVideoOverlay overlay;
        VideoStream stream(logger, audio_feed, video_feed, history, overlay);
        BallReader reader(stream);
        std::string result = reader.read_ball(images[0]);
        if (result == m_expected){
            return true;
        }
        return "Expected: " + m_expected + ", received: " + result;
    }

    std::string m_expected;
};

void add_tests_BallReader(UnitTestDatabase& database){
    database.add<Test_BallReader>("abra_1_deu.png", "master-ball");
    database.add<Test_BallReader>("bulbasaur_1_eng.png", "poke-ball");
    database.add<Test_BallReader>("bulbasaur_1_fra.png", "poke-ball");
    database.add<Test_BallReader>("bulbasaur_1_jpn.png", "poke-ball");
    database.add<Test_BallReader>("bulbasaur_2_eng.png", "poke-ball");
    database.add<Test_BallReader>("bulbasaur_3_eng.png", "poke-ball");
    database.add<Test_BallReader>("bulbasaur_4_eng.png", "poke-ball");
    database.add<Test_BallReader>("deoxys_1_jpn.png", "master-ball");
    database.add<Test_BallReader>("horsea_1_deu.png", "master-ball");
    database.add<Test_BallReader>("horsea_2_deu.png", "master-ball");
    database.add<Test_BallReader>("horsea_3_deu.png", "master-ball");
    database.add<Test_BallReader>("magikarp_1_spa.png", "poke-ball");
    database.add<Test_BallReader>("moltres_1_eng.png", "master-ball");
    database.add<Test_BallReader>("nidoran_m_eng.png", "ultra-ball");
    database.add<Test_BallReader>("pidgey_1_ita.png", "poke-ball");
    database.add<Test_BallReader>("venonat_4k_eng.png", "master-ball");
}


}
}
}
