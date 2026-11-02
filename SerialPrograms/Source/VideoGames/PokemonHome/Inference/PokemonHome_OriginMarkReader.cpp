/*  Origin Mark Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <vector>
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/DetectedBoxes.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Tests/ImageUnitTest.h"
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
            {0xff47503d, 0xff909090},
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

const MarkIconMatcher& MarkIcon_GameBoyAdvance(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/gba.png",
        "gameboyadvance",
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
        &MarkIcon_GameBoyAdvance(),
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
    case OriginMark::GAMEBOYADVANCE:
        return MarkIcon_GameBoyAdvance();
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

OriginMarkReader::OriginMarkReader(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(0.617, 0.084, 0.044, 0.069)
{}

void OriginMarkReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

OriginMark OriginMarkReader::read_mark(const ImageViewRGB32& original_screen){
    const double screen_rel_size = (original_screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 400;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    ImageViewRGB32 image = extract_box_reference(original_screen, m_box);

    m_last_detected.clear();
    m_last_detected_box.reset();

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
                        translate_to_parent(original_screen, m_box, object)
                    }
                );
                return true;
            }
        );

        if (!m_last_detected.empty()){
            if (m_overlay){
                m_last_detected_box.emplace(*m_overlay, m_box, COLOR_GREEN);
            }
            return origin_mark_from_slug(m_last_detected[0].name);
        }
    }

    return OriginMark::NONE;
}

class Test_OriginMarkReader : public ImageUnitTest{
public:

    Test_OriginMarkReader(
        const std::string& image,
        OriginMark expected
    )
        : ImageUnitTest("PokemonHome::OriginMarkReader - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const ImageRGB32& image = images[0];
        OriginMark result = OriginMarkReader().read_mark(image);

        if (result == m_expected)
            return true;

        return "Expected: " + ORIGIN_MARK_SLUGS().get_string(m_expected) + ", received: " + ORIGIN_MARK_SLUGS().get_string(result);
    };

private:
    OriginMark m_expected;
};

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
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", OriginMark::GAMEBOYADVANCE);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", OriginMark::GAMEBOYADVANCE);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", OriginMark::LA);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", OriginMark::GALAR);
    database.add<Test_OriginMarkReader>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", OriginMark::GALAR);
}


}
}
}