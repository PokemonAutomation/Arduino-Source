/*  Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <span>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "Tests/ImageUnitTest.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG_TypeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using Pokemon::PokemonType;


namespace{


struct TypeSlot{
    ImageFloatBox top;
    ImageFloatBox bottom;
};

struct TypeLayout{
    TypeSlot primary;
    TypeSlot secondary;
};

//  Sample above and below the lettering in each icon.
const TypeLayout INTERNATIONAL_LAYOUT{
    {
        {0.7083, 0.321875, 0.1083, 0.0046875},
        {0.7083, 0.384375, 0.1083, 0.0046875},
    },
    {
        {0.8625, 0.321875, 0.1083, 0.0046875},
        {0.8625, 0.384375, 0.1083, 0.0046875},
    },
};
const TypeLayout JAPANESE_LAYOUT{
    {
        {0.72083, 0.29140625, 0.10416, 0.0046875},
        {0.72083, 0.3546875, 0.10416, 0.0046875},
    },
    {
        {0.87083, 0.29140625, 0.1, 0.0046875},
        {0.87083, 0.3546875, 0.1, 0.0046875},
    },
};

struct ColorRange{
    Color min;
    Color max;

    bool contains(Color color) const{
        return min.red() <= color.red() && color.red() <= max.red()
            && min.green() <= color.green() && color.green() <= max.green()
            && min.blue() <= color.blue() && color.blue() <= max.blue();
    }
};

constexpr ColorRange TAN[] = {
    {{148, 151, 94}, {225, 222, 138}}
};
constexpr ColorRange RED[] = {
    {{179, 0, 0}, {255, 57, 26}}
};
constexpr ColorRange BLUE[] = {
    {{75, 119, 185}, {118, 181, 255}}
};
constexpr ColorRange CYAN[] = {
    {{126, 191, 187}, {182, 255, 255}}
};
constexpr ColorRange GREEN[] = {
    {{97, 164, 48}, {141, 255, 96}}
};
constexpr ColorRange PINK[] = {
    {{185, 47, 110}, {255, 118, 164}}
};
constexpr ColorRange TEAL[] = {
    {{55, 103, 111}, {91, 154, 172}}
};
constexpr ColorRange ORANGE[] = {
    {{190, 105, 0}, {215, 142, 64}},
    {{216, 105, 0}, {255, 156, 64}},
};
constexpr ColorRange PURPLE[] = {
    {{119, 20, 120}, {181, 80, 220}},
    {{182, 20, 167}, {212, 80, 222}},
};
constexpr ColorRange AMBER[] = {
    {{192, 158, 0}, {215, 181, 25}},
    {{216, 157, 0}, {239, 199, 16}},
    {{240, 158, 0}, {255, 222, 31}},
};
constexpr ColorRange YELLOW[] = {
    {{188, 205, 0}, {227, 255, 64}},
    {{228, 223, 0}, {255, 255, 64}},
};
constexpr ColorRange OCHRE[] = {
    {{162, 143, 26}, {215, 199, 71}},
    {{216, 175, 17}, {239, 204, 71}},
};

struct TypeColors{
    PokemonType type;
    std::span<const ColorRange> top;
    std::span<const ColorRange> bottom;
};

constexpr TypeColors TYPE_COLORS[] = {
    {PokemonType::NORMAL,   TAN,    TAN},
    {PokemonType::FIRE,     ORANGE, ORANGE},
    {PokemonType::FIGHTING, RED,    RED},
    {PokemonType::WATER,    BLUE,   BLUE},
    {PokemonType::FLYING,   CYAN,   TAN},
    {PokemonType::GRASS,    GREEN,  GREEN},
    {PokemonType::POISON,   PINK,   PURPLE},
    {PokemonType::ELECTRIC, AMBER,  AMBER},
    {PokemonType::GROUND,   YELLOW, OCHRE},
    {PokemonType::PSYCHIC,  PINK,   PINK},
    {PokemonType::ROCK,     OCHRE,  OCHRE},
    {PokemonType::ICE,      CYAN,   CYAN},
    {PokemonType::BUG,      YELLOW, YELLOW},
    {PokemonType::DRAGON,   BLUE,   RED},
    {PokemonType::GHOST,    PURPLE, PURPLE},
    {PokemonType::DARK,     TEAL,   TEAL},
    {PokemonType::STEEL,    TAN,    TEAL},
};

constexpr double MAX_STRIP_STDDEV_SUM = 24;

bool matches_color(Color color, std::span<const ColorRange> ranges){
    for (const ColorRange& range : ranges){
        if (range.contains(color)){
            return true;
        }
    }
    return false;
}

PokemonType read_slot(const ImageViewRGB32& game, const TypeSlot& slot){
    const ImageStats top = image_stats(extract_box_reference(game, slot.top));
    const ImageStats bottom = image_stats(extract_box_reference(game, slot.bottom));
    if (top.count == 0 || bottom.count == 0
        || top.stddev.sum() > MAX_STRIP_STDDEV_SUM || bottom.stddev.sum() > MAX_STRIP_STDDEV_SUM
    ){
        return PokemonType::NONE;
    }

    const Color top_color = top.average.round();
    const Color bottom_color = bottom.average.round();
    for (const TypeColors& colors : TYPE_COLORS){
        if (matches_color(top_color, colors.top) && matches_color(bottom_color, colors.bottom)){
            return colors.type;
        }
    }
    return PokemonType::NONE;
}

}


std::pair<PokemonType, PokemonType> TypeReader::read(Language language, const ImageViewRGB32& screen) const{
    if (!screen || !SummaryDetector(COLOR_RED).detect(screen)){
        return {PokemonType::NONE, PokemonType::NONE};
    }

    const ImageViewRGB32 game = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    const TypeLayout& layout = language == Language::Japanese ? JAPANESE_LAYOUT : INTERNATIONAL_LAYOUT;
    const PokemonType primary = read_slot(game, layout.primary);
    if (primary == PokemonType::NONE){
        return {PokemonType::NONE, PokemonType::NONE};
    }
    return {primary, read_slot(game, layout.secondary)};
}


class Test_TypeReader : public ImageUnitTest{
public:
    Test_TypeReader(
        const std::string& image, Language language,
        PokemonType primary, PokemonType secondary = PokemonType::NONE
    )
        : ImageUnitTest(
            "PokemonFRLG::TypeReader - " + image + " - " + language_data(language).code,
            {"PokemonFRLG/StatsReader/" + image}
        )
        , m_language(language)
        , m_expected(primary, secondary)
    {}

private:
    UnitTestResult run_on_images(Logger&, CancellableScope&, const std::vector<ImageRGB32>& images) const override{
        const auto actual = TypeReader().read(m_language, images[0]);
        if (actual == m_expected){
            return true;
        }
        const auto& slugs = Pokemon::POKEMON_TYPE_SLUGS();
        return "Expected: " + slugs.get_string(m_expected.first) + "/" + slugs.get_string(m_expected.second)
            + ", received: " + slugs.get_string(actual.first) + "/" + slugs.get_string(actual.second);
    }

    Language m_language;
    std::pair<PokemonType, PokemonType> m_expected;
};

void add_tests_TypeReader(UnitTestDatabase& database){
    using enum Language;
    using enum PokemonType;
    database.add<Test_TypeReader>("Page1/abra_1_deu.png", German, PSYCHIC);
    database.add<Test_TypeReader>("Page1/arbok_hd60pro_eng.png", English, POISON);
    database.add<Test_TypeReader>("Page1/arcanine_hd60pro_eng.png", English, FIRE);
    database.add<Test_TypeReader>("Page1/blastoise_hd60pro_eng.png", English, WATER);
    database.add<Test_TypeReader>("Page1/bulbasaur_1_eng.png", English, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/bulbasaur_1_fra.png", French, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/bulbasaur_1_jpn.png", Japanese, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/bulbasaur_2_eng.png", English, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/bulbasaur_3_eng.png", English, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/bulbasaur_4_eng.png", English, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/butterfree_hd60pro_eng.png", English, BUG, FLYING);
    database.add<Test_TypeReader>("Page1/deoxys_1_jpn.png", Japanese, PSYCHIC);
    database.add<Test_TypeReader>("Page1/diglett_hd60pro_eng.png", English, GROUND);
    database.add<Test_TypeReader>("Page1/dragonite_hd60pro_eng.png", English, DRAGON, FLYING);
    database.add<Test_TypeReader>("Page1/dragonite_usb3_eng.png", English, DRAGON, FLYING);
    database.add<Test_TypeReader>("Page1/ekans_hd60pro_eng.png", English, POISON);
    database.add<Test_TypeReader>("Page1/exeggcute_hd60pro_eng.png", English, GRASS, PSYCHIC);
    database.add<Test_TypeReader>("Page1/fearow_hd60pro_eng.png", English, NORMAL, FLYING);
    database.add<Test_TypeReader>("Page1/gengar_usb3_eng.png", English, GHOST, POISON);
    database.add<Test_TypeReader>("Page1/geodude_usb3_eng.png", English, ROCK, GROUND);
    database.add<Test_TypeReader>("Page1/gloom_hd60pro_eng.png", English, GRASS, POISON);
    database.add<Test_TypeReader>("Page1/golbat_hd60pro_eng.png", English, POISON, FLYING);
    database.add<Test_TypeReader>("Page1/horsea_1_deu.png", German, WATER);
    database.add<Test_TypeReader>("Page1/horsea_2_deu.png", German, WATER);
    database.add<Test_TypeReader>("Page1/horsea_3_deu.png", German, WATER);
    database.add<Test_TypeReader>("Page1/machoke_usb3_eng.png", English, FIGHTING);
    database.add<Test_TypeReader>("Page1/magikarp_1_spa.png", Spanish, WATER);
    database.add<Test_TypeReader>("Page1/magmar_usb3_eng.png", English, FIRE);
    database.add<Test_TypeReader>("Page1/magnemite_usb3_eng.png", English, ELECTRIC, STEEL);
    database.add<Test_TypeReader>("Page1/magneton_usb3_eng.png", English, ELECTRIC, STEEL);
    database.add<Test_TypeReader>("Page1/mantine_usb3_eng.png", English, WATER, FLYING);
    database.add<Test_TypeReader>("Page1/mewtwo_usb3_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/moltres_1_eng.png", English, FIRE, FLYING);
    database.add<Test_TypeReader>("Page1/mr-mime_hd60pro_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/nidoking_hd60pro_eng.png", English, POISON, GROUND);
    database.add<Test_TypeReader>("Page1/nidoqueen_hd60pro_eng.png", English, POISON, GROUND);
    database.add<Test_TypeReader>("Page1/nidoran_m_eng.png", English, POISON);
    database.add<Test_TypeReader>("Page1/onix_hd60pro_eng.png", English, ROCK, GROUND);
    database.add<Test_TypeReader>("Page1/parasect_hd60pro_eng.png", English, BUG, GRASS);
    database.add<Test_TypeReader>("Page1/pidgeotto_hd60pro_eng.png", English, NORMAL, FLYING);
    database.add<Test_TypeReader>("Page1/pidgey_1_ita.png", Italian, NORMAL, FLYING);
    database.add<Test_TypeReader>("Page1/psyduck_hd60pro_eng.png", English, WATER);
    database.add<Test_TypeReader>("Page1/raichu_hd60pro_eng.png", English, ELECTRIC);
    database.add<Test_TypeReader>("Page1/rhydon_usb3_eng.png", English, GROUND, ROCK);
    database.add<Test_TypeReader>("Page1/sneasel_usb3_eng.png", English, DARK, ICE);
    database.add<Test_TypeReader>("Page1/snorlax_hd60pro_eng.png", English, NORMAL);
    database.add<Test_TypeReader>("Page1/swinub_usb3_eng.png", English, ICE, GROUND);
    database.add<Test_TypeReader>("Page1/unown_c_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/unown_ex_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/unown_h_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/unown_j_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/unown_n_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/unown_p_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/unown_qu_eng.png", English, PSYCHIC);
    database.add<Test_TypeReader>("Page1/venonat_4k_eng.png", English, BUG, POISON);
    database.add<Test_TypeReader>("Page1/weedle_hd60pro_eng.png", English, BUG, POISON);
}


}
}
}
