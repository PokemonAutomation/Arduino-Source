/*  Max Lair Detect Battle Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/CancellableScope.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ColorClustering.h"
#include "Pokemon/Inference/Pokemon_ReadHpBar.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Detect_HPPP.h"
#include "PokemonSwSh_MaxLair_Detect_BattleMenu.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


BattleMenuDetector::BattleMenuDetector()
    : VisualInferenceCallback("BattleMenuDetector")
    , m_icon_fight  (0.923, 0.576 + 1 * 0.1075, 0.05, 0.080)
    , m_icon_pokemon(0.923, 0.576 + 2 * 0.1075, 0.05, 0.080)
    , m_icon_run    (0.923, 0.576 + 3 * 0.1075, 0.05, 0.080)
    , m_text_fight  (0.830, 0.576 + 1 * 0.1075, 0.08, 0.080)
    , m_text_pokemon(0.830, 0.576 + 2 * 0.1075, 0.08, 0.080)
    , m_text_run    (0.830, 0.576 + 3 * 0.1075, 0.08, 0.080)
    , m_icon_cheer  (0.923, 0.636 + 1 * 0.1075, 0.05, 0.020)
//    , m_info_left   (0.907, 0.500, 0.02, 0.03)
//    , m_info_right  (0.970, 0.500, 0.02, 0.03)
    , m_status0     (0.280, 0.870, 0.015, 0.030)
    , m_status1     (0.165, 0.945, 0.100, 0.015)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_YELLOW, m_icon_fight);
    items.add(COLOR_YELLOW, m_icon_pokemon);
    items.add(COLOR_YELLOW, m_icon_run);
    items.add(COLOR_YELLOW, m_text_fight);
    items.add(COLOR_YELLOW, m_text_pokemon);
    items.add(COLOR_YELLOW, m_text_run);
    items.add(COLOR_YELLOW, m_icon_cheer);
    items.add(COLOR_YELLOW, m_status0);
    items.add(COLOR_YELLOW, m_status1);
}
bool BattleMenuDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Need 5 consecutive successful detections.
    if (!detect(frame)){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    return m_trigger_count >= 5;
}


bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    bool fight;

    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_fight),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_fight),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
    );
    if (!fight){
//        cout << "Fight out" << endl;
        return false;
    }

    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_pokemon),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_pokemon),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
    );
    if (!fight){
//        cout << "Pokemon out" << endl;
        return false;
    }

    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_run),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_run),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
    );
    if (!fight){
//        cout << "Run out" << endl;
        return false;
    }

//    cout << "===============" << endl;

    fight = false;
    fight |= !fight && cluster_fit_2(   //  Not selected (red on white)
        extract_box_reference(screen, m_icon_fight),
        Color(255, 255, 255), 1.7,
        Color(153, 75, 112), 1.0
    );
    fight |= !fight && cluster_fit_2(   //  Selected (red on black)
        extract_box_reference(screen, m_icon_fight),
        Color(0, 0, 0), 1.4,
        Color(185, 6, 40), 1.0
    );
    fight |= !fight && cluster_fit_2(   //  Max raid Fight button is a bit different.
        extract_box_reference(screen, m_icon_fight),
        Color(0, 0, 0), 1.7,
        Color(182, 33, 82), 1.0
    );
//    cout << "===============" << endl;
    if (!fight){
        fight = cluster_fit_2(   //  Cheer
            extract_box_reference(screen, m_icon_cheer),
            Color(0, 0, 0), 1.0,
            Color(9, 162, 218), 1.0,
            0.2, 60
        );
//        cout << "fight = " << fight << endl;
        m_cheer = fight;
    }
    if (!fight){
//        cout << "Failed: m_icon_fight" << endl;
        return false;
    }

    bool pokemon = false;
    pokemon |= !pokemon && cluster_fit_2(
        extract_box_reference(screen, m_icon_pokemon),
        Color(255, 255, 255), 3.1,
        Color(126, 224, 142), 1.0
    );
    pokemon |= !pokemon && cluster_fit_2(
        extract_box_reference(screen, m_icon_pokemon),
        Color(0, 0, 0), 2.7,
        Color(8, 158, 18), 1.0
    );
    if (!pokemon){
//        cout << "Failed: m_icon_pokemon" << endl;
        return false;
    }

    bool run = false;
    run |= !run && cluster_fit_2(
        extract_box_reference(screen, m_icon_run),
        Color(255, 255, 255), 2.3,
        Color(216, 150, 230), 1.0
    );
    run |= !run && cluster_fit_2(
        extract_box_reference(screen, m_icon_run),
        Color(0, 0, 0), 1.9,
        Color(179, 15, 195), 1.0
    );
    if (!run){
//        cout << "Failed: m_icon_run" << endl;
        return false;
    }


    //  Check for white status bar in bottom left corner.
    ImageStats status = image_stats(extract_box_reference(screen, m_status0));
    ImageStats health = image_stats(extract_box_reference(screen, m_status1));
//    extract_box_reference(screen, m_status1).save("test.png");
//    cout << status.average << ", " << status.stddev << endl;
//    cout << health.average << ", " << health.stddev << endl;
    if (is_white(status, 500, 20) && is_white(health)){
        m_dmaxed = false;
        return true;
    }

    //  Check the semi-transparent red status bar if you're dmaxed.
    if (is_solid(status, {0.618001, 0.145809, 0.23619}, 0.15, 40) &&
        is_solid(health, {0.615249, 0.102789, 0.281963}, 0.15, 40)
    ){
        m_dmaxed = true;
        return true;
    }


//    image.save("battle-menu.png");
    return false;
}




BattleMenuReader::BattleMenuReader(
    VideoOverlay& overlay,
    Language language,
    OcrFailureWatchdog& ocr_watchdog
)
    : m_language(language)
    , m_ocr_watchdog(ocr_watchdog)
    , m_opponent_name(overlay, {0.3, 0.010, 0.4, 0.10}, COLOR_BLUE)
    , m_summary_opponent_name(overlay, {0.200, 0.100, 0.300, 0.065}, COLOR_BLUE)
    , m_summary_opponent_types(overlay, {0.200, 0.170, 0.300, 0.050}, COLOR_BLUE)
    , m_own_name(overlay, {0.060, 0.860, 0.160, 0.045}, COLOR_BLUE)
    , m_own_sprite(overlay, {0.002, 0.860, 0.060, 0.100}, COLOR_BLUE)
    , m_opponent_hp(overlay, {0.360, 0.120, 0.280, 0.005}, COLOR_BLUE)
    , m_own_hp(overlay, {0.069, 0.914, 0.204, 0.006}, COLOR_BLUE)
    , m_hp0(overlay, {0.073, 0.096 + 0*0.096, 0.052, 0.005}, COLOR_BLUE)
    , m_hp1(overlay, {0.073, 0.096 + 1*0.096, 0.052, 0.005}, COLOR_BLUE)
    , m_hp2(overlay, {0.073, 0.096 + 2*0.096, 0.052, 0.005}, COLOR_BLUE)
    , m_sprite0(overlay, {0.010, 0.040 + 0*0.096, 0.052, 0.061}, COLOR_BLUE)
    , m_sprite1(overlay, {0.010, 0.040 + 1*0.096, 0.052, 0.061}, COLOR_BLUE)
    , m_sprite2(overlay, {0.010, 0.040 + 2*0.096, 0.052, 0.061}, COLOR_BLUE)
    , m_pp0(overlay, {0.902, 0.710 - 1*0.097, 0.070, 0.063}, COLOR_BLUE)
    , m_pp1(overlay, {0.902, 0.710 + 0*0.097, 0.070, 0.063}, COLOR_BLUE)
    , m_pp2(overlay, {0.902, 0.710 + 1*0.097, 0.070, 0.063}, COLOR_BLUE)
    , m_pp3(overlay, {0.902, 0.710 + 2*0.097, 0.070, 0.063}, COLOR_BLUE)
    , m_dmax(overlay, {0.541, 0.779, 0.105, 0.186}, COLOR_BLUE)
{}

std::set<std::string> BattleMenuReader::read_opponent(
    Logger& logger, CancellableScope& scope,
    VideoFeed& feed
) const{
    std::set<std::string> result;

    VideoSnapshot screen;
    for (size_t c = 0; c < 3; c++){
        screen = feed.snapshot();
        ImageViewRGB32 image = extract_box_reference(screen, m_opponent_name);
        result = read_pokemon_name(logger, m_language, m_ocr_watchdog, image);
        if (!result.empty()){
            return result;
        }
        logger.log("Failed to read opponent name. Retrying in 1 second...", COLOR_ORANGE);
        scope.wait_for(std::chrono::seconds(1));
    }
//    dump_image(logger, MODULE_NAME, "MaxLair-read_opponent", screen);
    return result;
}
std::set<std::string> BattleMenuReader::read_opponent_in_summary(Logger& logger, const ImageViewRGB32& screen) const{
    //  Start by reading the types.
    PokemonType type0, type1;
    {
        ImageViewRGB32 types = extract_box_reference(screen, m_summary_opponent_types);
        std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = find_type_symbols(screen, types, 0.2);

        std::string type_str = "Type Read Result:\n";
        for (const auto& item : candidates){
            type_str += "    " + POKEMON_TYPE_SLUGS().get_string(item.second.first) + " : " + tostr_default(item.first ) + "\n";
        }
        logger.log(type_str);

        type0 = PokemonType::NONE;
        type1 = PokemonType::NONE;
        {
            auto iter = candidates.begin();
            if (iter != candidates.end()){
                type0 = iter->second.first;
                ++iter;
            }
            if (iter != candidates.end()){
                type1 = iter->second.first;
                ++iter;
            }
        }
    }

    //  Compile a list of all possible mons that match the type.
    std::set<std::string> allowed_slugs;
    for (const auto& item : maxlair_slugs()){
        const MaxLairMon& mon = get_maxlair_mon(item.first);
        if ((type0 == mon.type[0] && type1 == mon.type[1]) ||
            (type0 == mon.type[1] && type1 == mon.type[0])
        ){
            allowed_slugs.insert(item.first);
        }
    }

    //  Special case for stunfisk-galar which changes types.
    if (type1 == PokemonType::NONE){
        switch (type0){
        case PokemonType::ELECTRIC:
        case PokemonType::GRASS:
        case PokemonType::FAIRY:
        case PokemonType::PSYCHIC:
            allowed_slugs.insert("stunfisk-galar");
            break;
        default:;
        }
    }

    if (allowed_slugs.size() == 1){
        return allowed_slugs;
    }


    //  Now we read the name.
    std::set<std::string> name_slugs;
    {
        ImageViewRGB32 name = extract_box_reference(screen, m_summary_opponent_name);

        //  We can use a weaker threshold here since we are cross-checking with the type.
        name_slugs = read_pokemon_name(logger, m_language, m_ocr_watchdog, name, -1.0);
    }

    //  See if there's anything in common between the slugs that match the type
    //  and the slugs that match the OCR'ed name.
    std::set<std::string> common_slugs;
    for (const std::string& slug : name_slugs){
        if (allowed_slugs.find(slug) != allowed_slugs.end()){
            common_slugs.insert(slug);
        }
    }

#if 0
    for (const auto& slug : allowed_slugs){
        cout << "allowed_slugs = " << slug << endl;
    }
    for (const auto& slug : name_slugs){
        cout << "name_slugs = " << slug << endl;
    }
#endif

    if (common_slugs.size() == 1){
        logger.log("Disambiguation succeeded: " + *common_slugs.begin(), COLOR_BLUE);
        return common_slugs;
    }


    //  Special case: Korean Clefairy
    //  Reason: Korean OCR cannot read the character: 삐
    if (m_language == Language::Korean &&
        name_slugs.empty() &&
        type0 == PokemonType::FAIRY &&
        type1 == PokemonType::NONE
    ){
        logger.log("Known case that cannot be read: Korean Clefairy", COLOR_RED);
        return {"clefairy"};
    }



    static std::set<std::string> KNOWN_BAD_SLUGS{
        "basculin-blue-striped",
        "basculin-red-striped",
        "lycanroc-midday",
        "lycanroc-midnight",
//        "stunfisk-galar",   //  After using terrain pulse.
    };
    bool error = true;
    for (const std::string& slug : common_slugs){
        auto iter = KNOWN_BAD_SLUGS.find(slug);
        if (iter != KNOWN_BAD_SLUGS.end()){
            error = false;
            logger.log("Known case that cannot be disambiguated: (" + slug + ") Skipping error report.", COLOR_RED);
            break;
        }
    }

    //  At this point we're out of options.
    if (error){
        dump_image(logger, MODULE_NAME, "DisambiguateBoss", screen);
    }

    return common_slugs;
}
std::string BattleMenuReader::read_own_mon(Logger& logger, const ImageViewRGB32& screen) const{
    return read_pokemon_name_sprite(
        logger,
        m_ocr_watchdog,
        screen,
        m_own_sprite,
        m_own_name, m_language,
        false
    );
}

double BattleMenuReader::read_opponent_hp(Logger& logger, const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_opponent_hp);
//    image.save("test.png");

//    ImageStats stats = image_stats(image);
//    cout << stats.average << " - " << stats.stddev << endl;

#if 0
    double hp = read_hp_bar(image);
    logger.log("Opponent HP: " + (hp < 0 ? "?" : std::to_string(100 * hp)) + "%");
    if (hp < 0){
        dump_image(logger, screen, "BattleMenuReader-read_opponent_hp");
    }
    return hp;
#endif
    return read_hp_bar(logger, image);
}
double BattleMenuReader::read_own_hp(Logger& logger, const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_own_hp);
//    image.save("test.png");
#if 0
    double hp = read_hp_bar(image);
    logger.log("Your HP: " + (hp < 0 ? "?" : std::to_string(100 * hp)) + "%");
    if (hp == 0){
        hp = 0.001;
    }
    if (hp < 0){
        dump_image(logger, screen, "BattleMenuReader-read_own_hp");
    }
#endif
    return read_hp_bar(logger, image);
}
void BattleMenuReader::read_hp(Logger& logger, const ImageViewRGB32& screen, Health health[4], size_t player_index){
    Health tmp_hp[4];
    tmp_hp[0] = {read_own_hp(logger, screen), false};
    tmp_hp[1] = read_in_battle_hp_box(logger, extract_box_reference(screen, m_sprite0), extract_box_reference(screen, m_hp0));
    tmp_hp[2] = read_in_battle_hp_box(logger, extract_box_reference(screen, m_sprite1), extract_box_reference(screen, m_hp1));
    tmp_hp[3] = read_in_battle_hp_box(logger, extract_box_reference(screen, m_sprite2), extract_box_reference(screen, m_hp2));
    bool bad = false;
    for (size_t c = 0; c < 4; c++){
        bad |= tmp_hp[c].hp < 0;
        health[(c + player_index) % 4] = tmp_hp[c];
    }

    if (bad){
//        dump_image(logger, MODULE_NAME, "BattlePartyReader-ReadHP", screen);
    }
}
void BattleMenuReader::read_own_pp(Logger& logger, const ImageViewRGB32& screen, int8_t pp[4]) const{
    pp[0] = read_pp_text(logger, extract_box_reference(screen, m_pp0));
    pp[1] = read_pp_text(logger, extract_box_reference(screen, m_pp1));
    pp[2] = read_pp_text(logger, extract_box_reference(screen, m_pp2));
    pp[3] = read_pp_text(logger, extract_box_reference(screen, m_pp3));
    if (pp[0] < 0 && pp[1] < 0 && pp[2] < 0 && pp[3] < 0){
//        dump_image(logger, MODULE_NAME, "BattleMenuReader-read_own_pp", screen);
        return;
    }
#if 0
    bool ok = pp[0] > 0 || pp[1] > 0 || pp[2] > 0 || pp[3] > 0;
    if (ok){
        for (size_t c = 0; c < 4; c++){
            pp[c] = std::max(pp[c], (int8_t)0);
        }
    }
#endif
}


bool dmax_circle_ready(const ImageViewRGB32& image){
    size_t w = image.width();
    size_t h = image.height();
    if (w * h <= 1){
        return false;
    }

    w = 200;
    h = 200;
    ImageRGB32 processed = image.scale_to(w, h);
    if (image_stats(processed).stddev.sum() < 75){
        return false;
    }
//    cout << image_stats(processed).stddev.sum() << endl;


    size_t total = 0;
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (size_t r = 0; r < h; r++){
        for (size_t c = 0; c < w; c++){
            int dy = (int)r - 100;
            int dx = (int)c - 100;
            if (dy < -60){
//                processed.setPixelColor(c, r, COLOR_BLUE);
                continue;
            }
            if (-25 < dy && dy < 55){
//                processed.setPixelColor(c, r, COLOR_BLUE);
                continue;
            }
            if (dx*dx + dy*dy < 72*72){
//                processed.setPixelColor(c, r, COLOR_BLUE);
                continue;
            }
            if (dx*dx + dy*dy > 80*80){
//                processed.setPixelColor(c, r, COLOR_BLUE);
                continue;
            }
            FloatPixel p(processed.pixel(c, r));
            total++;
            sum += p;
            sqr_sum += p * p;
        }
    }
//    processed.save("test.png");

//    size_t total = (size_t)w * (size_t)h;
    double totalf = (double)total;
    FloatPixel variance = (sqr_sum - sum*sum / totalf) / (totalf - 1);
    ImageStats stats{
        sum / totalf,
        FloatPixel(
            std::sqrt(variance.r),
            std::sqrt(variance.g),
            std::sqrt(variance.b)
        ),
        total
    };
//    cout << stats.average << stats.stddev << endl;

    if (stats.average.r < 128){
        return false;
    }
    return is_solid(stats, {0.684591, 0.000481775, 0.314928}, 0.15, 50);

//    ImageStats stats = image_stats(processed);
}
bool BattleMenuReader::can_dmax(const ImageViewRGB32& screen) const{
    return dmax_circle_ready(extract_box_reference(screen, m_dmax));
}

























}
}
}
}
