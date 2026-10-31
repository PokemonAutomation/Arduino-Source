/*  Tera Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "Tests/ImageUnitTest.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonHome_GigantamaxDetector.h"
#include "PokemonHome_TeraTypeReader.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonHome {

using namespace Kernels;
using namespace Kernels::Waterfill;
using Pokemon::PokemonTeraType;
using Pokemon::POKEMON_TERA_TYPE_SLUGS;

namespace {

class TypeSprite {
public:
    TypeSprite(const std::string& slug, const std::string& resource_location)
        : m_slug(slug)
    {
        ImageRGB32 sprite(RESOURCE_PATH() + resource_location);

        sprite = filter_rgb32_range(
            sprite,
            0xff000000, 0xffffffff,
            Color(0), false
        );

        PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(sprite, 224, 224, 224);
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 10);

        WaterfillObject object;
        for (const WaterfillObject& item : objects){
            object.merge_assume_no_overlap(item);
        }

        m_aspect_ratio = object.aspect_ratio();
        m_matcher = std::make_unique<ImageMatch::WeightedExactImageMatcher>(
            sprite.sub_image(object.min_x, object.min_y, object.width(), object.height()).copy(),
            ImageMatch::WeightedExactImageMatcher::InverseStddevWeight{ 1, 64 }
        );
    }

    const std::string& slug() const { return m_slug; }
    double aspect_ratio() const { return m_aspect_ratio; }
    const ImageMatch::WeightedExactImageMatcher& matcher() const { return *m_matcher; }

private:
    std::string m_slug;
    double m_aspect_ratio;
    std::unique_ptr<ImageMatch::WeightedExactImageMatcher> m_matcher;
};

struct TeraTypeSpriteDatabase {
    std::map<PokemonTeraType, TypeSprite> m_type_map;

    static TeraTypeSpriteDatabase& instance(){
        static TeraTypeSpriteDatabase data;
        return data;
    }

    TeraTypeSpriteDatabase(){
        for (const auto& item : POKEMON_TERA_TYPE_SLUGS()){
            if (item.first == PokemonTeraType::NONE){
                continue;
            }
            m_type_map.emplace(item.first, TypeSprite(item.second, "PokemonHome/TeraTypes/" + item.second + ".png"));
        }
    }
};

size_t distance_sqr(const ImagePixelBox& a, const ImagePixelBox& b){
    bool overlap_x = a.min_x <= b.max_x && b.min_x <= a.max_x;
    bool overlap_y = a.min_y <= b.max_y && b.min_y <= a.max_y;
    if (overlap_x && overlap_y){
        return 0;
    }

    size_t dist_x = 0;
    if (!overlap_x){
        dist_x = a.max_x < b.min_x
            ? b.min_x - a.max_x
            : a.min_x - b.max_x;
    }

    size_t dist_y = 0;
    if (!overlap_y){
        dist_y = a.max_y < b.min_y
            ? b.min_y - a.max_y
            : a.min_y - b.max_y;
    }

    return dist_x * dist_x + dist_y * dist_y;
}

bool image_validation(const ImageViewRGB32& image){
    size_t width = image.width();
    size_t height = image.height();
    if (width * height < 100){
        return false;
    }
    if (width > 3 * height){
        return false;
    }
    if (height > 3 * width){
        return false;
    }
    ImageStats stats = image_stats(image);
    if (stats.stddev.sum() < 50){
        return false;
    }

    return true;
}

std::pair<double, PokemonTeraType> match_tera_type_symbol(const ImageViewRGB32& image){
    if (!image_validation(image)){
        return { 1.0, PokemonTeraType::NONE };
    }

    size_t width = image.width();
    size_t height = image.height();

    double aspect_ratio = (double)width / height;

    const std::map<PokemonTeraType, TypeSprite>* type_sprite_map = &TeraTypeSpriteDatabase::instance().m_type_map;

    double best_score = 0.45;
    PokemonTeraType best_type = PokemonTeraType::NONE;
    for (const auto& item : *type_sprite_map){
        double expected_aspect_ratio = item.second.aspect_ratio();
        double ratio = aspect_ratio / expected_aspect_ratio;

        if (std::abs(ratio - 1) > 0.2){
            continue;
        }

        double rmsd_alpha = item.second.matcher().diff(image);

        if (best_score > rmsd_alpha){
            best_score = rmsd_alpha;
            best_type = item.first;
        }
    }

    return { best_score, best_type };
}

void find_tera_type_symbol_candidates(
    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>>& candidates,
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image,
    PackedBinaryMatrix& matrix,
    double max_area_ratio
){
    size_t max_area = (size_t)(image.width() * image.height() * max_area_ratio);
    std::vector<WaterfillObject> objects = find_objects_inplace(
        matrix,
        (size_t)(18. * original_screen.total_pixels() / (1920 * 1080))
    );

    std::map<size_t, WaterfillObject> objmap;
    for (size_t c = 0; c < objects.size(); c++){
        if (objects[c].area > max_area){
            continue;
        }
        objmap[c] = objects[c];
    }

    bool changed;
    do{
        changed = false;
        for (auto iter0 = objmap.begin(); iter0 != objmap.end(); ++iter0){
            for (auto iter1 = objmap.begin(); iter1 != objmap.end();){
                if (iter0->first >= iter1->first){
                    ++iter1;
                    continue;
                }
                const WaterfillObject& obj0 = iter0->second;
                const WaterfillObject& obj1 = iter1->second;
                size_t distance = distance_sqr(
                    ImagePixelBox(obj0.min_x, obj0.min_y, obj0.max_x, obj0.max_y),
                    ImagePixelBox(obj1.min_x, obj1.min_y, obj1.max_x, obj1.max_y)
                );
                if (distance < 5 * 5){
                    iter0->second.merge_assume_no_overlap(iter1->second);
                    iter1 = objmap.erase(iter1);
                    changed = true;
                }else{
                    ++iter1;
                }
            }
        }
    }
    while (changed);

    for (const auto& item : objmap){
        ImageViewRGB32 img = extract_box_reference(image, item.second);

        std::pair<double, PokemonTeraType> result = match_tera_type_symbol(img);
        if (result.second != PokemonTeraType::NONE){
            const WaterfillObject& obj = item.second;
            candidates.emplace(
                result.first,
                std::pair<PokemonTeraType, ImagePixelBox>(
                    result.second,
                    ImagePixelBox(obj.min_x, obj.min_y, obj.max_x, obj.max_y)
                )
            );
        }
    }
}

}

std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> find_tera_type_symbols(
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image,
    double max_area_ratio
){
    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> candidates;

    {
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            image,
            {
                {0xff808060, 0xffffffff},
                {0xffa0a060, 0xffffffff},
                {0xff606060, 0xffffffff},
                {0xff707070, 0xffffffff},
                {0xff808080, 0xffffffff},
                {0xff909090, 0xffffffff},
                {0xffa0a0a0, 0xffffffff},
                {0xffb0b0b0, 0xffffffff},
                {0xffc0c0c0, 0xffffffff},
                {0xffd0d0d0, 0xffffffff},
                {0xffd8d8d8, 0xffffffff},
                {0xffe0e0e0, 0xffffffff},
            }
                );
        for (PackedBinaryMatrix& matrix : matrices){
            find_tera_type_symbol_candidates(candidates, original_screen, image, matrix, max_area_ratio);
        }
    }

    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> filtered;
    for (const auto& candidate : candidates){
        bool is_dupe = false;
        for (const auto& item : filtered){
            if (distance_sqr(candidate.second.second, item.second.second) == 0){
                is_dupe = true;
                break;
            }
        }
        if (!is_dupe){
            filtered.emplace(candidate);
        }
    }

    return filtered;
}

PokemonTeraType read_pokemon_tera_type(
    const ImageViewRGB32& original_screen,
    const ImageFloatBox& box
){
    GigantamaxDetector gmax_detector(COLOR_RED, nullptr, box);
    if (gmax_detector.detect(original_screen)){
        return PokemonTeraType::NONE;
    }

    ImageViewRGB32 image = extract_box_reference(original_screen, box);

    std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> filtered = find_tera_type_symbols(
        original_screen,
        image,
        0.20
    );

    if (filtered.empty()){
        return PokemonTeraType::NONE;
    }

    return filtered.begin()->second.first;
}

class Test_TeraTypeReader : public ImageUnitTest{
public:

    Test_TeraTypeReader(
        const std::string& image,
        PokemonTeraType expected
    )
        : ImageUnitTest("PokemonHome::TeraTypeReader - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const ImageRGB32& image = images[0];
        PokemonTeraType result = read_pokemon_tera_type(image);
        
        if (result == m_expected)
            return true;

        return "Expected: " + POKEMON_TERA_TYPE_SLUGS().get_string(m_expected) + ", received: " + POKEMON_TERA_TYPE_SLUGS().get_string(result);
    };

private:
    PokemonTeraType m_expected;
};

//TODO: Missing multiple positive test cases for TeraTypeReader.
void add_tests_TeraTypeReader(UnitTestDatabase& database){
    database.add<Test_TeraTypeReader>("PokemonHome/BoxView/BoxView-1.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/annihilape_Regular.png", PokemonTeraType::ICE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/bidoof_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/capskid_Regular.png", PokemonTeraType::GRASS);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/castform_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/cyclizar_Regular.png", PokemonTeraType::FAIRY);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", PokemonTeraType::DARK);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", PokemonTeraType::NORMAL);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/enamorus_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", PokemonTeraType::GHOST);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/glimmet_Regular.png", PokemonTeraType::ELECTRIC);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gogoat_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", PokemonTeraType::STEEL);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/hatterne_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/houndstone_Regular.png", PokemonTeraType::GROUND);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ironBunde_Regular.png", PokemonTeraType::WATER);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", PokemonTeraType::WATER);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", PokemonTeraType::FLYING);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ironThorns_Regular.png", PokemonTeraType::ROCK);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", PokemonTeraType::POISON);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/kingler_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/komala_Regular.png", PokemonTeraType::PSYCHIC);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/krabby_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/machamp_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/pancham_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rapidash_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rellor_Regular.png", PokemonTeraType::BUG);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/riolu_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/scovillain_Regular.png", PokemonTeraType::FIRE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", PokemonTeraType::FIGHTING);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/squirtle_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", PokemonTeraType::DRAGON);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/teddiursa_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/terapagos_regular.png", PokemonTeraType::STELLAR);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/vulpix_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/vulpix_Shiny.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/wartortle_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/wurmple_Regular.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", PokemonTeraType::PSYCHIC);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", PokemonTeraType::FLYING);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", PokemonTeraType::POISON);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", PokemonTeraType::GRASS);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", PokemonTeraType::WATER);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", PokemonTeraType::FIRE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", PokemonTeraType::WATER);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", PokemonTeraType::NONE);
    database.add<Test_TeraTypeReader>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", PokemonTeraType::NONE);

}

}
}
}