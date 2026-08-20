/*  Origin Mark Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <vector>
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/DetectedBoxes.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonHome_OriginMarkReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

using Pokemon::ORIGIN_MARK_SLUGS;

class MarkIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MarkIconMatcher(
        const char* path,
        std::string name,
        Color min_color, Color max_color,
        size_t min_area,
        double max_rmsd,
        std::vector<std::pair<uint32_t, uint32_t>> filters,
        double area_ratio_override = 0
    )
        : WaterfillTemplateMatcher(path, min_color, max_color, min_area)
        , m_name(std::move(name))
        , m_max_rmsd(max_rmsd)
        , m_filters(std::move(filters))
    {
        if (area_ratio_override != 0){
            m_area_ratio = area_ratio_override;
        }
    }

    const std::string& name() const{
        return m_name;
    }
    double max_rmsd() const{
        return m_max_rmsd;
    }
    const std::vector<std::pair<uint32_t, uint32_t>>& filters() const{
        return m_filters;
    }

private:
    std::string m_name;
    double m_max_rmsd;
    std::vector<std::pair<uint32_t, uint32_t>> m_filters;
};

const MarkIconMatcher& MarkIcon_Kalos(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/kalos.png",
        "kalos",
        Color(0xff61645c), Color(0xff646664),
        10,
        60.0,
        {
            {0xff61645c, 0xff646664},
            {0xff49523f, 0xff898e7e}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Alola(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/alola.png",
        "alola",
        Color(0xff5f6456), Color(0xff6d706b),
        10,
        35.0,
        {
            {0xff5f6456, 0xff6d706b},
            {0xff3e4733, 0xffa5ae99}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_GameBoy(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/gameboy.png",
        "gameboy",
        Color(0xff61645b), Color(0xff646664),
        10,
        60.0,
        {
            {0xff61645b, 0xff646664},
            {0xff424c3c, 0xffb7c6a9}
        }
        );
    return ret;
}

const MarkIconMatcher& MarkIcon_Go(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/go.png",
        "go",
        Color(0xff5c644e), Color(0xff798073),
        10,
        110.0,
        {
            {0xff5c644e, 0xff798073},
            {0xff364029, 0xffc5d5b5}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Lgpe(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/lgpe.png",
        "lgpe",
        Color(0xff616459), Color(0xff676d64),
        10,
        45.0,
        {
            {0xff616459, 0xff676d64},
            {0xff3e4832, 0xff85907c}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Galar(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/galar.png",
        "galar",
        Color(0xff5e6453), Color(0xff757870),
        10,
        60.0,
        {
            {0xff5e6453, 0xff757870},
            {0xff47503d, 0xfffffff8}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Bdsp(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/bdsp.png",
        "bdsp",
        Color(0xff5f6457), Color(0xff767972),
        10,
        60.0,
        {
            {0xff5f6457, 0xff767972},
            {0xff3d4731, 0xffb6c6ad}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_La(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/la.png",
        "la",
        Color(0xff606459), Color(0xff686a65),
        10,
        90.0,
        {
            {0xff606459, 0xff686a65},
            {0xff3a462e, 0xffe4f4ce}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Sv(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/sv.png",
        "sv",
        Color(0xff5d6450), Color(0xff747771),
        10,
        90,
        {
            {0xff5d6450, 0xff747771},
            {0xff38422c, 0xfff7ffd7}
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Lza(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/lza.png",
        "lza",
        Color(0xff61645b), Color(0xff646764),
        10,
        70.0,
        {
            {0xff61645b, 0xff646764},
            {0xff424c3c, 0xffb7c6a9}
        }
    );
    return ret;
}

const std::vector<const MarkIconMatcher*>& ALL_ORIGIN_MARK_MATCHERS(){
    static const std::vector<const MarkIconMatcher*> matchers = {
        &MarkIcon_Kalos(),
        &MarkIcon_Alola(),
        &MarkIcon_GameBoy(),
        &MarkIcon_Go(),
        &MarkIcon_Lgpe(),
        &MarkIcon_Galar(),
        &MarkIcon_Bdsp(),
        &MarkIcon_La(),
        &MarkIcon_Sv(),
        &MarkIcon_Lza(),
    };
    return matchers;
}

const MarkIconMatcher& get_mark_icon_matcher(OriginMark mark){
    switch (mark){
    case OriginMark::KALOS:
        return MarkIcon_Kalos();
    case OriginMark::ALOLA:
        return MarkIcon_Alola();
    case OriginMark::GAMEBOY:
        return MarkIcon_GameBoy();
    case OriginMark::GO:
        return MarkIcon_Go();
    case OriginMark::LGPE:
        return MarkIcon_Lgpe();
    case OriginMark::GALAR:
        return MarkIcon_Galar();
    case OriginMark::BDSP:
        return MarkIcon_Bdsp();
    case OriginMark::LA:
        return MarkIcon_La();
    case OriginMark::SV:
        return MarkIcon_Sv();
    case OriginMark::LZA:
        return MarkIcon_Lza();
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid origin mark: " + std::to_string((int)mark));
    }
}

OriginMark origin_mark_from_slug(const std::string& slug){
    for (const auto& item : Pokemon::ORIGIN_MARK_SLUGS()){
        if (item.second == slug){
            return item.first;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid origin mark slug: " + slug);
}

OriginMark OriginMarkReader::read_mark(
    const ImageViewRGB32& original_screen,
    const ImageFloatBox& box
){
    const double screen_rel_size = (original_screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 400;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    ImageViewRGB32 image = extract_box_reference(original_screen, box);

    m_last_detected.clear();

    std::multimap<double, std::pair<OriginMark, ImagePixelBox>> candidates;

    for (const MarkIconMatcher* matcher : ALL_ORIGIN_MARK_MATCHERS()){
        match_template_by_waterfill(
            original_screen.size(),
            image,
            *matcher,
            matcher->filters(),
            { min_area, SIZE_MAX },
            matcher->max_rmsd(),
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                
                m_last_detected.emplace_back(
                    DetectedBox{
                        matcher->name(),
                        translate_to_parent(original_screen, box, object)
                    }
                );
                return true;
            }
        );

        if (!m_last_detected.empty()){
            return origin_mark_from_slug(m_last_detected[0].name);
        }
    }

    return OriginMark::NONE;
}

class Test_OriginMarkReader : public UnitTest{
public:

    Test_OriginMarkReader(
        const std::string& image,
        OriginMark expected
    )
        : UnitTest("PokemonHome::OriginMarkReader - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        OriginMark result = OriginMarkReader().read_mark(image, ImageFloatBox(0.617, 0.084, 0.044, 0.069));

        if (result == m_expected)
            return true;

        return "Expected: " + ORIGIN_MARK_SLUGS().get_string(m_expected) + ", received: " + ORIGIN_MARK_SLUGS().get_string(result);
    };

private:
    std::string m_image;
    OriginMark m_expected;
};

//TODO: Missing multiple positive test cases for TeraTypeReader.
void add_tests_OriginMarkReader(UnitTestDatabase& database){
    database.add<Test_OriginMarkReader>("PokemonHome/BoxView/BoxView-1.png", OriginMark::NONE);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/annihilape_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/bidoof_Regular.png", OriginMark::NONE);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", OriginMark::LGPE);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", OriginMark::GO);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", OriginMark::LZA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/capskid_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/castform_Regular.png", OriginMark::GO);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/cyclizar_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/enamorus_Shiny.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/glimmet_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gogoat_Regular.png", OriginMark::KALOS);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/hatterne_Regular.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/houndstone_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ironBunde_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ironThorns_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/kingler_Shiny.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/komala_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/krabby_Shiny.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/machamp_Regular.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/pancham_Shiny.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rapidash_Regular.png", OriginMark::LGPE);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rellor_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/riolu_Regular.png", OriginMark::LZA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/scovillain_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/squirtle_Shiny.png", OriginMark::GO);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", OriginMark::ALOLA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/teddiursa_Regular.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/terapagos_regular.png", OriginMark::SV);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/vulpix_Regular.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/vulpix_Shiny.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/wartortle_Regular.png", OriginMark::GO);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/wurmple_Regular.png", OriginMark::BDSP);
}


}
}
}