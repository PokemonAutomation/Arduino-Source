/*  Pokemon FRLG Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonFRLG_ShinySymbolDetector.h"
#include "VideoGames/PokemonFRLG/Resources/PokemonFRLG_PokemonSprites.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PokemonSpriteReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

//  Positions are in GBA pixels. The GBA screen is 240 x 160.
constexpr int GBA_WIDTH = 240;
constexpr int GBA_HEIGHT = 160;
constexpr int SPRITE_SIZE = 64;

ImageFloatBox sprite_box(int x, int y){
    return ImageFloatBox(
        (double)x / GBA_WIDTH, (double)y / GBA_HEIGHT,
        (double)SPRITE_SIZE / GBA_WIDTH, (double)SPRITE_SIZE / GBA_HEIGHT
    );
}

// When the summary is opened, the sprite bounces up by as much as 9 pixels.
// the following allow 1 extra pixel of slack in each direction.
constexpr int SUMMARY_X_MIN = 27;
constexpr int SUMMARY_X_MAX = 31;
constexpr int SUMMARY_Y_MIN = 23;
constexpr int SUMMARY_Y_MAX = 34;

//  In battle, the sprite's top edge depends on the species and whether it floats.
constexpr int BATTLE_X = 144;
constexpr int BATTLE_Y_MIN = 0;
constexpr int BATTLE_Y_MAX = 36;

}



PositionedSpriteMatcher::PositionedSpriteMatcher(
    const std::set<std::string>* subset,
    bool flipped,                               // most sprites are flipped on the summary screen
    const std::vector<bool>& shiny_variants,    // {false, true} to match against both normal and shiny sprites
    int x_min, int x_max, int y_min, int y_max
)
    : m_x_min(x_min), m_x_max(x_max)
    , m_y_min(y_min), m_y_max(y_max)
{
    for (bool shiny : shiny_variants){
        for (const auto& item : POKEMON_SPRITES(shiny, flipped)){
            if (subset != nullptr && !sprite_in_species_subset(item.first, *subset)){
                continue;
            }
            m_templates.emplace_back(new Template{
                sprite_slug_to_species(item.first),
                shiny,
                ImageMatch::WeightedExactImageMatcher(item.second.sprite.copy(), {1, 256})
            });
        }
    }
}

ImageFloatBox PositionedSpriteMatcher::search_box() const{
    return ImageFloatBox(
        (double)m_x_min / GBA_WIDTH, (double)m_y_min / GBA_HEIGHT,
        (double)(m_x_max - m_x_min + SPRITE_SIZE) / GBA_WIDTH,
        (double)(m_y_max - m_y_min + SPRITE_SIZE) / GBA_HEIGHT
    );
}

PositionedSpriteMatcher::Result PositionedSpriteMatcher::match(
    const ImageViewRGB32& game_screen, double alpha_spread
) const{
    Result ret;
    if (!game_screen || m_templates.empty()){
        return ret;
    }

    //  Every possible sprite position scaled to the template size
    std::vector<ImageRGB32> candidates;
    for (int y = m_y_min; y <= m_y_max; y++){
        for (int x = m_x_min; x <= m_x_max; x++){
            candidates.emplace_back(
                extract_box_reference(game_screen, sprite_box(x, y)).scale_to(SPRITE_SIZE, SPRITE_SIZE)
            );
        }
    }

    std::map<std::string, double> species_best;
    double best_alpha = 10000;
    for (const std::unique_ptr<Template>& item : m_templates){
        double alpha = 10000;
        for (const ImageRGB32& candidate : candidates){
            alpha = std::min(alpha, item->matcher.diff(candidate));
        }
        auto iter = species_best.emplace(item->species, alpha).first;
        iter->second = std::min(iter->second, alpha);
        if (alpha < best_alpha){
            best_alpha = alpha;
            ret.shiny = item->shiny;
        }
    }
    for (const auto& item : species_best){
        ret.species.add(item.second, item.first);
    }
    ret.species.clear_beyond_spread(alpha_spread);
    return ret;
}



SummarySpriteReader::SummarySpriteReader(const std::set<std::string>& subset, Color color, bool shiny)
    : m_color(color)
    , m_matcher(
        &subset, true, {shiny},
        SUMMARY_X_MIN, SUMMARY_X_MAX, SUMMARY_Y_MIN, SUMMARY_Y_MAX
    )
{}

void SummarySpriteReader::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_matcher.search_box()));
}

ImageMatch::ImageMatchResult SummarySpriteReader::read(const ImageViewRGB32& frame) const{
    static const double ALPHA_SPREAD = 0.10;
    if (!frame){
        return ImageMatch::ImageMatchResult();
    }
    ImageViewRGB32 game_screen =
        extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    return m_matcher.match(game_screen, ALPHA_SPREAD).species;
}



BattleSpriteReader::BattleSpriteReader(const std::set<std::string>* subset, Color color)
    : m_color(color)
    , m_matcher(
        subset, false, {false, true},
        BATTLE_X, BATTLE_X, BATTLE_Y_MIN, BATTLE_Y_MAX
    )
{}

void BattleSpriteReader::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_matcher.search_box()));
}

BattleSpriteReader::Result BattleSpriteReader::read(const ImageViewRGB32& frame) const{
    static const double ALPHA_SPREAD = 0.10;
    if (!frame){
        return Result();
    }
    ImageViewRGB32 game_screen =
        extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    return m_matcher.match(game_screen, ALPHA_SPREAD);
}



class Test_SummarySpriteReader : public ImageUnitTest{
public:
    Test_SummarySpriteReader(const std::string& image, std::string expected)
        : ImageUnitTest("PokemonFRLG::SummarySpriteReader - " + image, {"PokemonFRLG/StatsReader/" + image})
        , m_expected(std::move(expected))
    {}

private:
    UnitTestResult run_on_images(
        Logger& logger, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        // Match against every species
        static const std::set<std::string> ALL_SLUGS = []{
            std::set<std::string> slugs;
            for (const auto& item : ALL_POKEMON_SPRITES()){
                slugs.insert(sprite_slug_to_species(item.first));
            }
            return slugs;
        }();

        //  Pick the normal or shiny sprites based on the shiny symbol
        bool shiny = ShinySymbolDetector(COLOR_YELLOW).read(logger, images[0]);
        SummarySpriteReader reader(ALL_SLUGS, COLOR_RED, shiny);
        ImageMatch::ImageMatchResult result = reader.read(images[0]);
        result.log(logger, 1.0);
        if (result.results.empty()){
            return "Expected: " + m_expected + ", received no match.";
        }
        const std::string& received = result.results.begin()->second;
        if (received == m_expected){
            return true;
        }
        return "Expected: " + m_expected + ", received: " + received;
    }

    std::string m_expected;
};

void add_tests_SummarySpriteReader(UnitTestDatabase& database){
    database.add<Test_SummarySpriteReader>("Page1/abra_1_deu.png", "abra");
    database.add<Test_SummarySpriteReader>("Page1/bulbasaur_1_eng.png", "bulbasaur");
    database.add<Test_SummarySpriteReader>("Page1/bulbasaur_1_fra.png", "bulbasaur");
    database.add<Test_SummarySpriteReader>("Page1/bulbasaur_1_jpn.png", "bulbasaur");
    database.add<Test_SummarySpriteReader>("Page1/bulbasaur_2_eng.png", "bulbasaur");
    database.add<Test_SummarySpriteReader>("Page1/bulbasaur_3_eng.png", "bulbasaur");
    database.add<Test_SummarySpriteReader>("Page1/bulbasaur_4_eng.png", "bulbasaur");
    database.add<Test_SummarySpriteReader>("Page1/deoxys_1_jpn.png", "deoxys");
    database.add<Test_SummarySpriteReader>("Page1/horsea_1_deu.png", "horsea");
    database.add<Test_SummarySpriteReader>("Page1/horsea_2_deu.png", "horsea");
    database.add<Test_SummarySpriteReader>("Page1/horsea_3_deu.png", "horsea");
    database.add<Test_SummarySpriteReader>("Page1/magikarp_1_spa.png", "magikarp");
    database.add<Test_SummarySpriteReader>("Page1/moltres_1_eng.png", "moltres");
    database.add<Test_SummarySpriteReader>("Page1/nidoran_m_eng.png", "nidoran-m");
    database.add<Test_SummarySpriteReader>("Page1/pidgey_1_ita.png", "pidgey");
    database.add<Test_SummarySpriteReader>("Page1/unown_c_eng.png", "unown-c");
    database.add<Test_SummarySpriteReader>("Page1/unown_ex_eng.png", "unown-!");
    database.add<Test_SummarySpriteReader>("Page1/unown_h_eng.png", "unown-h");
    database.add<Test_SummarySpriteReader>("Page1/unown_j_eng.png", "unown-j");
    database.add<Test_SummarySpriteReader>("Page1/unown_n_eng.png", "unown-n");
    database.add<Test_SummarySpriteReader>("Page1/unown_p_eng.png", "unown-p");
    database.add<Test_SummarySpriteReader>("Page1/unown_qu_eng.png", "unown-?");
    database.add<Test_SummarySpriteReader>("Page1/venonat_4k_eng.png", "venonat");
    database.add<Test_SummarySpriteReader>("Page2/abra_1_deu.png", "abra");
    database.add<Test_SummarySpriteReader>("Page2/deoxys_1_jpn.png", "deoxys");
    database.add<Test_SummarySpriteReader>("Page2/mewtwo_1_spa.png", "mewtwo");
    database.add<Test_SummarySpriteReader>("Page2/nidoranf_1_eng.jpg", "nidoran-f");
    database.add<Test_SummarySpriteReader>("Page2/raikou_1_eng.png", "raikou");
    database.add<Test_SummarySpriteReader>("Page2/raikou_2_eng.png", "raikou");
    database.add<Test_SummarySpriteReader>("Page2/scyther_1_eng.png", "scyther");
    database.add<Test_SummarySpriteReader>("Page2/venonat_1_eng.jpg", "venonat");
}


class Test_BattleSpriteReader : public ImageUnitTest{
public:
    Test_BattleSpriteReader(const std::string& image, std::string expected, bool shiny)
        : ImageUnitTest("PokemonFRLG::BattleSpriteReader - " + image, {"PokemonFRLG/WildEncounterReader/" + image})
        , m_expected(std::move(expected))
        , m_shiny(shiny)
    {}

private:
    UnitTestResult run_on_images(
        Logger& logger, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        //  Match against every species, normal and shiny.
        static const BattleSpriteReader reader;
        BattleSpriteReader::Result result = reader.read(images[0]);
        result.species.log(logger, 1.0);
        if (result.species.results.empty()){
            return "Expected: " + m_expected + ", received no match.";
        }
        const std::string& received = result.species.results.begin()->second;
        if (received != m_expected){
            return "Expected: " + m_expected + ", received: " + received;
        }
        if (result.shiny != m_shiny){
            return std::string("Expected ") + (m_shiny ? "shiny" : "not shiny") +
                   ", received " + (result.shiny ? "shiny" : "not shiny") + ".";
        }
        return true;
    }

    std::string m_expected;
    bool m_shiny;
};

void add_tests_BattleSpriteReader(UnitTestDatabase& database){
    database.add<Test_BattleSpriteReader>("eng_articuno.jpg", "articuno", true);
    database.add<Test_BattleSpriteReader>("eng_chansey.jpg", "chansey", true);
    database.add<Test_BattleSpriteReader>("eng_chansey_dark.png", "chansey", false);
    database.add<Test_BattleSpriteReader>("eng_dragonair.jpg", "dragonair", true);
    database.add<Test_BattleSpriteReader>("eng_heracross.jpg", "heracross", true);
    database.add<Test_BattleSpriteReader>("eng_ho-oh.jpg", "ho-oh", true);
    database.add<Test_BattleSpriteReader>("eng_kakuna.jpg", "kakuna", true);
    database.add<Test_BattleSpriteReader>("eng_machop.jpg", "machop", true);
    database.add<Test_BattleSpriteReader>("eng_pikachu.jpg", "pikachu", true);
    database.add<Test_BattleSpriteReader>("eng_unown_c.png", "unown-c", false);
    database.add<Test_BattleSpriteReader>("eng_unown_ex.jpg", "unown-!", true);
    database.add<Test_BattleSpriteReader>("eng_unown_h.png", "unown-h", false);
    database.add<Test_BattleSpriteReader>("eng_unown_j.png", "unown-j", false);
    database.add<Test_BattleSpriteReader>("eng_unown_n.png", "unown-n", false);
    database.add<Test_BattleSpriteReader>("eng_unown_p.png", "unown-p", false);
    database.add<Test_BattleSpriteReader>("eng_unown_qu.jpg", "unown-?", true);
    database.add<Test_BattleSpriteReader>("eng_unown_s.png", "unown-s", false);
    database.add<Test_BattleSpriteReader>("fra_snorlax.jpg", "snorlax", true);
}



}
}
}
