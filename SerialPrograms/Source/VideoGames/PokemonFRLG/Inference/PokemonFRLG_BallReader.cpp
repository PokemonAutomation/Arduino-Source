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
        : ImageUnitTest("PokemonFRLG::BallReader - " + image, {image})
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
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/abra_1_deu.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_fra.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_jpn.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/bulbasaur_2_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/bulbasaur_3_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/bulbasaur_4_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/deoxys_1_jpn.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/horsea_1_deu.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/horsea_2_deu.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/horsea_3_deu.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/magikarp_1_spa.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/moltres_1_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/nidoran_m_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/pidgey_1_ita.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_c_eng.png", "timer-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_ex_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_h_eng.png", "net-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_j_eng.png", "luxury-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_n_eng.png", "repeat-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_p_eng.png", "nest-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/unown_qu_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/venonat_4k_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/arbok_hd60pro_eng.png", "great-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/arcanine_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/blastoise_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/butterfree_hd60pro_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/diglett_hd60pro_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/dragonite_hd60pro_eng.png", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/dragonite_usb3_eng.png", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/ekans_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/exeggcute_hd60pro_eng.png", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/fearow_hd60pro_eng.png", "great-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/gengar_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/geodude_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/gloom_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/golbat_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/machoke_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/magmar_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/magnemite_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/magneton_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/mantine_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/mewtwo_usb3_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/mr-mime_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/nidoking_hd60pro_eng.png", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/nidoqueen_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/onix_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/parasect_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/pidgeotto_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/psyduck_hd60pro_eng.png", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/raichu_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/rhydon_usb3_eng.png", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/sneasel_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/snorlax_hd60pro_eng.png", "great-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/swinub_usb3_eng.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page1/weedle_hd60pro_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/abra_1_deu.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/deoxys_1_jpn.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/mewtwo_1_spa.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/nidoranf_1_eng.jpg", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/raikou_1_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/raikou_2_eng.png", "master-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/scyther_1_eng.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/StatsReader/Page2/venonat_1_eng.jpg", "safari-ball");
    database.add<Test_BallReader>("PokemonFRLG/ShinySymbolDetector/Char_False.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/ShinySymbolDetector/Karp-coloredited_True.png", "poke-ball");
    database.add<Test_BallReader>("PokemonFRLG/ShinySymbolDetector/Karp-snotyak_True.png", "poke-ball");
}


}
}
}
