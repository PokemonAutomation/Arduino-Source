/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "VideoGames/PokemonHome/Resources/PokemonHome_PokeballSprites.h"
#include "PokemonHome_BallReader.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

namespace{
    constexpr double MAX_BALL_ALPHA = 0.40;

    ImageViewRGB32 crop_ball_sprite(const ImageViewRGB32& image, double min_distance_squared){
        ImageStats border = image_border_stats(image);
        ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [&](Color pixel){
                double r = (double)pixel.red() - border.average.r;
                double g = (double)pixel.green() - border.average.g;
                double b = (double)pixel.blue() - border.average.b;
                return r*r + g*g + b*b >= min_distance_squared;
            }
        );
        return extract_box_reference(image, box);
    }
}



class PokeballSpriteMatcher::SimilarBallGroupMatcher{
    struct PairMatchers{
        PairMatchers(size_t first, size_t second, ImageRGB32 a, ImageRGB32 b)
            : first(first), second(second)
            , a(std::move(a), {1, 128})
            , b(std::move(b), {1, 128})
        {}
        size_t first;
        size_t second;
        ImageMatch::WeightedExactImageMatcher a;
        ImageMatch::WeightedExactImageMatcher b;
    };

public:
    SimilarBallGroupMatcher(
        const SimilarBallGroup& config,
        const std::map<std::string, ImageRGB32>& cleaned_sprites
    )
        : m_config(config)
    {
        if (config.slugs.size() < 2 || config.comparison_width == 0 || config.comparison_height == 0
            || config.min_detail_pixels < 2 || config.min_pixel_distance <= 0
            || config.crop_distance <= 0 || config.coarse_spread < 0
            || config.max_detail_alpha < 0 || config.detail_spread < 0
        ){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid similar ball group.");
        }

        std::vector<ImageRGB32> sprites;
        for (const std::string& slug : config.slugs){
            auto iter = cleaned_sprites.find(slug);
            if (iter == cleaned_sprites.end()){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid similar ball group member: " + slug);
            }
            ImageViewRGB32 trimmed = ImageMatch::trim_image_alpha(iter->second);
            sprites.emplace_back(trimmed.scale_to(config.comparison_width, config.comparison_height));
        }

        for (size_t i = 0; i < sprites.size(); i++){
            for (size_t j = 0; j < i; j++){
                ImageRGB32 a = sprites[j].copy();
                ImageRGB32 b = sprites[i].copy();
                size_t detail_pixels = 0;
                for (size_t y = 0; y < config.comparison_height; y++){
                    for (size_t x = 0; x < config.comparison_width; x++){
                        Color first(a.pixel(x, y)), second(b.pixel(x, y));
                        double r = (double)first.red() - second.red();
                        double g = (double)first.green() - second.green();
                        double blue = (double)first.blue() - second.blue();
                        if (first.alpha() == 255 && second.alpha() == 255
                            && r*r + g*g + blue*blue >= config.min_pixel_distance * config.min_pixel_distance
                        ){
                            detail_pixels++;
                        }else{
                            a.pixel(x, y) = 0;
                            b.pixel(x, y) = 0;
                        }
                    }
                }
                if (detail_pixels >= config.min_detail_pixels){
                    m_pairs.emplace_back(std::make_unique<PairMatchers>(j, i, std::move(a), std::move(b)));
                }
            }
        }
    }

    double coarse_spread() const{ return m_config.coarse_spread; }

    bool contains(const std::string& slug) const{
        return std::find(m_config.slugs.begin(), m_config.slugs.end(), slug) != m_config.slugs.end();
    }

    std::string winner(const ImageViewRGB32& image) const{
        ImageViewRGB32 crop = crop_ball_sprite(image, m_config.crop_distance * m_config.crop_distance);
        std::vector<size_t> wins(m_config.slugs.size(), 0);
        for (const auto& pair : m_pairs){
            double a = pair->a.diff(crop);
            double b = pair->b.diff(crop);
            if (a <= m_config.max_detail_alpha && b - a > m_config.detail_spread){
                wins[pair->first]++;
            }
            if (b <= m_config.max_detail_alpha && a - b > m_config.detail_spread){
                wins[pair->second]++;
            }
        }
        for (size_t i = 0; i < wins.size(); i++){
            if (wins[i] == m_config.slugs.size() - 1){
                return m_config.slugs[i];
            }
        }
        return "";
    }

private:
    SimilarBallGroup m_config;
    std::vector<std::unique_ptr<PairMatchers>> m_pairs;
};


PokeballSpriteMatcher::PokeballSpriteMatcher(double min_euclidean_distance)
    : PokeballSpriteMatcher(
        {
            SimilarBallGroup{
                .slugs = {"heavy-ball-pla", "leaden-ball", "gigaton-ball"},
            },
            SimilarBallGroup{
                .slugs = {"fast-ball", "poke-ball"},
            },
        },
        min_euclidean_distance
    )
{}
PokeballSpriteMatcher::PokeballSpriteMatcher(
    const std::vector<SimilarBallGroup>& groups, double min_euclidean_distance
)
    : CroppedImageDictionaryMatcher({1, 128})
    , m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
{
    std::map<std::string, ImageRGB32> cleaned_sprites;
    for (const auto& item : PokemonHome::ALL_POKEBALL_SPRITES()){
        ImageRGB32 sprite = remove_white_border(item.second.sprite);
        add(item.first, sprite);
        cleaned_sprites.emplace(item.first, std::move(sprite));
    }
    std::vector<std::string> grouped_slugs;
    for (const SimilarBallGroup& group : groups){
        for (const std::string& slug : group.slugs){
            if (std::find(grouped_slugs.begin(), grouped_slugs.end(), slug) != grouped_slugs.end()){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Overlapping similar ball groups: " + slug);
            }
            grouped_slugs.emplace_back(slug);
        }
        m_similar_groups.emplace_back(std::make_unique<SimilarBallGroupMatcher>(group, cleaned_sprites));
    }
}
PokeballSpriteMatcher::~PokeballSpriteMatcher() = default;

ImageMatch::ImageMatchResult PokeballSpriteMatcher::match(const ImageViewRGB32& image, double alpha_spread) const{
    double coarse_spread = alpha_spread;
    for (const auto& group : m_similar_groups){
        coarse_spread = std::max(coarse_spread, group->coarse_spread());
    }

    ImageMatch::ImageMatchResult coarse = CroppedImageDictionaryMatcher::match(image, coarse_spread);
    if (coarse.results.empty()){
        return coarse;
    }
    ImageMatch::ImageMatchResult result = coarse;
    result.clear_beyond_spread(alpha_spread);
    const double best_alpha = coarse.results.begin()->first;
    if (best_alpha <= MAX_BALL_ALPHA){
        for (const auto& group : m_similar_groups){
            bool plausible = false;
            for (const auto& item : coarse.results){
                if (group->contains(item.second) && item.first <= MAX_BALL_ALPHA
                    && item.first <= best_alpha + group->coarse_spread()
                ){
                    plausible = true;
                    break;
                }
            }
            if (!plausible){
                continue;
            }

            std::string winner = group->winner(image);
            auto selected = std::find_if(coarse.results.begin(), coarse.results.end(), [&](const auto& item){
                return item.second == winner && item.first <= MAX_BALL_ALPHA
                    && item.first <= best_alpha + group->coarse_spread();
            });
            if (selected == coarse.results.end()){
                continue;
            }

            for (auto iter = result.results.begin(); iter != result.results.end();){
                if (group->contains(iter->second)){
                    iter = result.results.erase(iter);
                }else{
                    ++iter;
                }
            }
            result.add(selected->first, winner);
        }
    }
    result.clear_beyond_spread(alpha_spread);
    return result;
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
    std::vector<ImageViewRGB32> ret;
    ret.emplace_back(crop_ball_sprite(image, m_min_euclidean_distance_squared));
    return ret;
}






const double BallReader::MAX_ALPHA = MAX_BALL_ALPHA;
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
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/bidoof_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", "repeat-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/capskid_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/castform_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/cyclizar_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/enamorus_Shiny.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/glimmet_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gogoat_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/hatterne_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/houndstone_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ironBunde_Regular.png", "dive-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ironThorns_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/kingler_Shiny.png", "net-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/komala_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/krabby_Shiny.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/machamp_Regular.png", "repeat-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/pancham_Shiny.png", "luxury-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rapidash_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rellor_Regular.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/riolu_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", "jet-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/scovillain_Regular.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", "beast-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/squirtle_Shiny.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/teddiursa_Regular.png", "wing-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/terapagos_regular.png", "luxury-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/vulpix_Regular.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/vulpix_Shiny.png", "luxury-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/wartortle_Regular.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/wurmple_Regular.png", "premier-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", "fast-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", "gigaton-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", "gigaton-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", "gigaton-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", "gigaton-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", "gigaton-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", "fast-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", "timer-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", "love-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", "love-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", "timer-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", "heal-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", "heal-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", "luxury-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", "nest-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", "premier-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", "heal-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", "repeat-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", "luxury-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", "premier-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", "nest-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", "love-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", "nest-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", "quick-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", "dive-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", "sport-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", "heal-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", "heal-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", "great-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", "repeat-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", "ultra-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", "cherish-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", "poke-ball");
    database.add<Test_BallReader>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", "poke-ball");

}



}
}
}
