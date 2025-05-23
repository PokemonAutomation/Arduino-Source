#include "PokemonHome_Enrichment.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonHome/Inference/PokemonHome_BoxGenderDetector.h"
#include "PokemonHome/Inference/PokemonHome_HomeApplicationDetector.h"
#include "PokemonHome/Programs/HomeEnvironment/PokemonHome_HomeEnvironment.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.h"
#include <iostream>
#include <qobject.h>
#include <unordered_set>
#include <cmath>
#include <stdexcept>
#include <ctime>
#include <windows.h>
#include <string>
#include <sstream>



namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;

const size_t MAX_COLUMNS = 6;
const size_t MAX_ROWS = 5;

Enrichment_Descriptor::Enrichment_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonHome:Enrichment",
          STRING_POKEMON + " Home", "Enrichment",
          "ComputerControl/blob/master/Wiki/Programs/PokemonHome/Enrichment.md",
          "Order boxes of " + STRING_POKEMON + ".",
          FeedbackType::REQUIRED,
          AllowCommandsWhenRunning::DISABLE_COMMANDS,
          {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
          )
{}
struct Enrichment_Descriptor::Stats : public StatsTracker{
    Stats()

    {

    }

};
std::unique_ptr<StatsTracker> Enrichment_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

Enrichment::Enrichment()
    : HOME_FIRST_BOX(
          "<b>First dedicated box in Home:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          1, 1, 200
          )
    , HOME_LAST_BOX(
          "<b>Last dedicated box in Home:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          1, 1, 200
          )
    , PLA_FIRST_BOX(
          "<b>First Dedicated box in Legends Arceus:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          16, 1, 32
          )
    , PLA_LAST_BOX(
          "<b>Last Dedicated box in Legends Arceus:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          28, 1, 32
          )
    , SV_BOX_NAME(
          false,
          "<b>S/V Box Name:</b><br>Name of the box in Scarlet/Violet to use for storage",
          LockMode::LOCK_WHILE_RUNNING,
          "To Home",
          "box_order"
          )
    , WIPE_MARKINGS(
          "<b>Wipe Markings:</b><br>Clear markings from home or pick up where the program last left off.",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
    , DISPOSE_GOS(
          "<b>Go Disposal:</b><br>Get rid of Pokémon with a Go origin mark (compatible with PLA). Will not include shinies",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
    , STARTING_AT_DESK(
          "<b>Starting at Desk:</b><br>Check this if you are absolutely sure the save is starting inside the<br>Uva Academy at the Academy Ace Tournament desk.",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
    , EMERGENCY_DELOAD(
          "<b>DEBUG Emergency Deload:</b><br>Dump Pokemon into Home",
          LockMode::UNLOCK_WHILE_RUNNING,
          false
          )
    , NORMAL_DELOAD(
          "<b>DEBUG Normal Deload:</b><br>Avoid Second marking on home movements",
          LockMode::UNLOCK_WHILE_RUNNING,
          false
          )
    , SKIP_SETUP(
          "<b>DEBUG Skip Setup:</b><br>Skip loading pokemon into home boxes",
          LockMode::UNLOCK_WHILE_RUNNING,
          false
          )
    , NOTIFICATIONS({
          &NOTIFICATION_PROGRAM_FINISH
      })
{
    PA_ADD_OPTION(HOME_FIRST_BOX);            //number of first box to check and sort in HOME
    PA_ADD_OPTION(HOME_LAST_BOX);             //number of last box to check and sort in HOME
    PA_ADD_OPTION(PLA_FIRST_BOX);            //number of first box to check and sort in PLA
    PA_ADD_OPTION(PLA_LAST_BOX);             //number of last box to check and sort in PLA
    PA_ADD_OPTION(SV_BOX_NAME);         //box to use for s/v
    PA_ADD_OPTION(WIPE_MARKINGS);        //should wipe markings on home
    PA_ADD_OPTION(DISPOSE_GOS);         //Dispose of Go marked Pokémon
    PA_ADD_OPTION(STARTING_AT_DESK);     //pokemon sv start at desk
    PA_ADD_OPTION(EMERGENCY_DELOAD);     //pokemon sv start at desk
    PA_ADD_OPTION(NORMAL_DELOAD);     //pokemon sv start at desk
    PA_ADD_OPTION(SKIP_SETUP);          //pokemon sv start at desk
    PA_ADD_OPTION(NOTIFICATIONS);

}

#include <unordered_set>
#include <string>
#include <sstream>
#include <stdexcept>
#include <memory>

class Pokemon {
public:
    float national_dex_number;
    bool shiny;
    bool gmax;
    StatsHuntGenderFilter gender;
    uint16_t level;
    size_t form_id;
    std::string type1;
    std::string type2;
    FloatPixel color;

    size_t current_box; // Box number the Pokémon is currently in
    size_t current_row; // Row position in the current box
    size_t current_col; // Column position in the current box

    // Default constructor
    Pokemon()
        : national_dex_number(0), shiny(false), gmax(false), gender(StatsHuntGenderFilter::Genderless),
        level(0), form_id(0), type1("None"), type2("None"), color(), current_box(0), current_row(0), current_col(0) {}

    // Copy constructor for safe cloning
    Pokemon(const Pokemon& other) = default;

    // Shared gender-specific IDs across all Pokemon
    static const std::unordered_set<float>& gender_specific_ids() {
        static const std::unordered_set<float> ids = {3.0f, 12.0f, 19.0f, 19.009995f, 20.0f, 25.0f, 26.0f, 41.0f, 42.0f, 44.0f, 45.0f, 64.0f, 65.0f, 84.0f, 85.0f, 97.0f, 111.0f, 112.0f, 118.0f, 119.0f, 123.0f, 129.0f, 130.0f, 133.0f, 154.0f, 165.0f, 166.0f, 178.0f, 185.0f, 186.0f, 190.0f, 194.0f, 195.0f, 198.0f, 202.0f, 203.0f, 207.0f, 208.0f, 212.0f, 214.0f, 215.0f, 215.009995f, 217.0f, 221.0f, 224.0f, 229.0f, 232.0f, 255.0f, 256.0f, 257.0f, 267.0f, 269.0f, 272.0f, 274.0f, 275.0f, 307.0f, 308.0f, 315.0f, 316.0f, 317.0f, 322.0f, 323.0f, 332.0f, 350.0f, 369.0f, 396.0f, 397.0f, 398.0f, 399.0f, 400.0f, 401.0f, 402.0f, 403.0f, 404.0f, 405.0f, 407.0f, 415.0f, 417.0f, 418.0f, 419.0f, 424.0f, 443.0f, 444.0f, 445.0f, 449.0f, 450.0f, 453.0f, 454.0f, 456.0f, 457.0f, 459.0f, 460.0f, 461.0f, 464.0f, 465.0f, 473.0f, 521.0f, 592.0f, 593.0f, 668.0f, 678.0f, 876.0f, 902.0f, 916.0f};
        return ids;
    }


    // Operator< for sorting Pokémon
    bool operator<(const Pokemon& other) const {
        if (shiny != other.shiny) {
            return shiny;
        }
        if (national_dex_number != other.national_dex_number) {
            return national_dex_number < other.national_dex_number;
        }
        if (gender_specific_ids().count(national_dex_number) > 0) {
            if (gender != other.gender) {
                return this->gender== StatsHuntGenderFilter::Female;
            }
        }
        if (level != other.level) {
            return level > other.level;
        }
        return false;
    }

    // Overload == for Pokemon (if not already defined)
    bool operator==(const Pokemon& other) const {
        if (gender_specific_ids().count(national_dex_number) > 0) {
            return shiny == other.shiny &&
                   national_dex_number == other.national_dex_number &&
                   level == other.level &&
                   gender == other.gender;
        }else{
            return shiny == other.shiny &&
                   national_dex_number == other.national_dex_number &&
                   level == other.level;
        }
    }

    bool operator<=(const Pokemon& other) const {
        return *this < other || *this == other;
    }

    void update_national_id(){
        static std::unordered_map<size_t, std::vector<std::pair<std::string, std::string>>> regional_codes = {
            {19, {{"Normal", "None"}, {"Dark", "Normal"}}},
            {20, {{"Normal", "None"}, {"Dark", "Normal"}}},
            {26, {{"Electric", "None"}, {"Electric", "Psychic"}}},
            {27, {{"Ground", "None"}, {"Ice", "Steel"}}},
            {28, {{"Ground", "None"}, {"Ice", "Steel"}}},
            {37, {{"Fire", "None"}, {"Ice", "None"}}},
            {38, {{"Fire", "None"}, {"Ice", "Fairy"}}},
            {50, {{"Ground", "None"}, {"Ground", "Steel"}}},
            {51, {{"Ground", "None"}, {"Ground", "Steel"}}},
            {52, {{"Normal", "None"}, {"Dark", "None"}, {"Steel", "None"}}},
            {53, {{"Normal", "None"}, {"Dark", "None"}}},
            {58, {{"Fire", "None"}, {"Fire", "Rock"}}},
            {59, {{"Fire", "None"}, {"Fire", "Rock"}}},
            {74, {{"Rock", "Ground"}, {"Rock", "Electric"}}},
            {75, {{"Rock", "Ground"}, {"Rock", "Electric"}}},
            {76, {{"Rock", "Ground"}, {"Rock", "Electric"}}},
            {77, {{"Fire", "None"}, {"Psychic", "None"}}},
            {78, {{"Fire", "None"}, {"Psychic", "Fairy"}}},
            {79, {{"Water", "Psychic"}, {"Psychic", "None"}}},
            {80, {{"Water", "Psychic"}, {"Poison", "Psychic"}}},
            {83, {{"Normal", "Flying"}, {"Fighting", "None"}}},
            {88, {{"Poison", "None"}, {"Poison", "Dark"}}},
            {89, {{"Poison", "None"}, {"Poison", "Dark"}}},
            {100, {{"Electric", "None"}, {"Electric", "Grass"}}},
            {101, {{"Electric", "None"}, {"Electric", "Grass"}}},
            {103, {{"Grass", "Psychic"}, {"Grass", "Dragon"}}},
            {105, {{"Ground", "None"}, {"Fire", "Ghost"}}},
            {110, {{"Poison", "None"}, {"Poison", "Fairy"}}},
            {122, {{"Psychic", "Fairy"}, {"Ice", "Psychic"}}},
            {128, {{"Normal", "None"}, {"Fighting", "None"}, {"Fighting", "Fire"}, {"Fighting", "Water"}}},
            {144, {{"Ice", "Flying"}, {"Psychic", "Flying"}}},
            {145, {{"Electric", "Flying"}, {"Fighting", "Flying"}}},
            {146, {{"Fire", "Flying"}, {"Dark", "Flying"}}},
            {157, {{"Fire", "None"}, {"Fire", "Ghost"}}},
            {194, {{"Water", "Ground"}, {"Poison", "Ground"}}},
            {199, {{"Water", "Psychic"}, {"Poison", "Psychic"}}},
            {211, {{"Water", "Poison"}, {"Dark", "Poison"}}},
            {215, {{"Dark", "Ice"}, {"Fighting", "Poison"}}},
            {222, {{"Water", "Rock"}, {"Ghost", "None"}}},
            {263, {{"Normal", "None"}, {"Dark", "Normal"}}},
            {264, {{"Normal", "None"}, {"Dark", "Normal"}}},
            {479, {{"Electric", "Ghost"}, {"Electric", "Fire"}, {"Electric", "Water"}, {"Electric", "Ice"}, {"Electric", "Flying"}, {"Electric", "Grass"}}},
            {492, {{"Grass", "None"}, {"Grass", "Flying"}}},
            {503, {{"Water", "None"}, {"Water", "Dark"}}},
            {549, {{"Grass", "None"}, {"Grass", "Fighting"}}},
            {554, {{"Fire", "None"}, {"Ice", "None"}}},
            {555, {{"Fire", "None"}, {"Ice", "None"}}},
            {556, {{"Fire", "Psychic"}, {"Ice", "Fire"}}},
            {562, {{"Ghost", "None"}, {"Ground", "Ghost"}}},
            {570, {{"Dark", "None"}, {"Normal", "Ghost"}}},
            {571, {{"Dark", "None"}, {"Normal", "Ghost"}}},
            {618, {{"Ground", "Electric"}, {"Ground", "Steel"}}},
            {628, {{"Normal", "Flying"}, {"Psychic", "Flying"}}},
            {648, {{"Normal", "Psychic"}, {"Normal", "Fighting"}}},
            {705, {{"Dragon", "None"}, {"Steel", "Dragon"}}},
            {706, {{"Dragon", "None"}, {"Steel", "Dragon"}}},
            {713, {{"Ice", "None"}, {"Ice", "Rock"}}},
            {724, {{"Grass", "Ghost"}, {"Grass", "Fighting"}}},
            {741, {{"Fire", "Flying"}, {"Electric", "Flying"}, {"Psychic", "Flying"}, {"Ghost", "Flying"}}}
            // Urshifu

        };

        static std::unordered_map<size_t, std::vector<std::pair<FloatPixel, FloatPixel>>> visual_forms = {
            // {25, {{}}} // Pikachu (Base+Shiny, Cap, Hoenn, Sinnoh, Unova, Kalos, Alola, Partner, World)
            {201, {{FloatPixel(170.934285, 172.982582, 171.41171), /*Need shiny A form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(171.051985, 172.886542, 171.652027), /*Need shiny B form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(176.412055, 178.22759, 176.8081), /*Need shiny C form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(159.567979, 161.690026, 160.454821), FloatPixel(157.188212, 198.651457, 236.569283)},
                    {FloatPixel(160.319388, 162.789918, 161.06639), /*Need shiny E form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(153.205915, 155.198765, 154.417376), FloatPixel(150.810034, 193.6098, 235.118375)},
                    {FloatPixel(182.024254, 183.959737, 182.321307), FloatPixel(180.19163, 211.312942, 238.684495)},
                    {FloatPixel(162.561158, 164.912954, 163.358196), /*Need shiny H form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(173.638146, 175.842502, 174.158502), /*Need shiny I form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(174.78719, 176.783337, 175.370398), FloatPixel(173.128448, 205.976991, 236.628388)},
                    {FloatPixel(155.181497, 157.789063, 156.005996), FloatPixel(152.902236, 195.905834, 234.782663)},
                    {FloatPixel(182.587585, 184.501769, 183.053828), FloatPixel(180.942259, 210.843551, 238.327138)},
                    {FloatPixel(165.695827, 167.787549, 166.355483), /*Need shiny M form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(156.977935, 159.123696, 157.581949), /*Need shiny N form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(168.583553, 170.585142, 169.121477), FloatPixel(166.553304, 203.350642, 236.466633)},
                    {FloatPixel(165.417406, 167.340598, 166.266998), /*Need shiny P form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(157.113683, 158.939366, 157.916402), FloatPixel(154.486959, 193.69863, 232.002593)},
                    {FloatPixel(150.893273, 152.667826, 151.728475), FloatPixel(148.442904, 191.789708, 233.815925)},
                    {FloatPixel(171.734725, 173.602096, 172.299721), FloatPixel(169.83889, 206.225567, 238.884264)},
                    {FloatPixel(183.944942, 185.595455, 184.330615), FloatPixel(182.191525, 213.09559, 240.793006)},
                    {FloatPixel(149.057921, 150.999191, 149.797233), /*Need shiny U form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(174.859411, 176.782708, 175.724967), FloatPixel(173.018827, 209.631461, 241.697251)},
                    {FloatPixel(174.45659, 176.309105, 174.934824), FloatPixel(172.27166, 207.564951, 239.244499)},
                    {FloatPixel(153.545989, 155.090164, 154.541672), /*Need shiny X form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(178.924631, 180.80897, 179.51523), /*Need shiny Y form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(161.410736, 163.732717, 162.055058), FloatPixel(159.08811, 198.56639, 234.025063)},
                    {FloatPixel(166.253478, 167.907453, 166.422323), /*Need shiny ! form Unown*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(162.93385, 164.838815, 163.50664), FloatPixel(160.774164, 205.105798, 242.707804)}}}, // Unown (A-Z?!+Shiny)
            // {351, {{}}} // Castform (Normal+Shiny, Sunny+Shiny, Rainy+Shiny, Snowy+Shiny)
            // {386, {{}}} // Deoxys (Normal+Shiny, Attack+Shiny, Defense+Shiny, Speed+Shiny)
            {412, {{FloatPixel(187.917961, 224.88263, 150.888176), FloatPixel(186.449005, 226.713005, 155.512037)},
                    {FloatPixel(234.216738, 234.237543, 209.198765), FloatPixel(232.057786, 236.224607, 215.107357)},
                    {FloatPixel(238.852021, 218.341228, 220.157693), FloatPixel(236.174496, 219.359965, 224.627593)}}}, // Burmy (Plant+Shiny, Sandy+Shiny, Trash+Shiny)
            {413, {{FloatPixel(167.571861, 212.879677, 158.623366), FloatPixel(167.632945, 215.726481, 164.212975)},
                    {FloatPixel(227.790158, 209.376304, 177.705915), FloatPixel(227.686578, 212.199439, 183.602096)},
                    {/*Need nonshiny Trash Cloak form Wormadam*/ FloatPixel(255, 255, 255), FloatPixel(240.343821, 199.945587, 213.737379)}}}, // Wormadam (Plant+Shiny, Sandy+Shiny, Trash+Shiny)
            {422, {{FloatPixel(123.741276, 214.299886, 216.025723), /*Need shiny East sea form Shellos*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(249.065206, 203.930327, 199.138116), /*Need shiny West sea form Shellos*/ FloatPixel(255, 255, 255)}}}, // Shellos (East Sea+Shiny, West Sea+Shiny)
            {423, {{FloatPixel(115.69842, 205.94094, 167.599727), /*Need shiny East sea form Gastrodon*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(223.847374, 184.596384, 159.035166), FloatPixel(221.540532, 199.802884, 136.430507)}}}, // Gastrodon (East Sea+Shiny, West Sea+Shiny)
            {550, {{FloatPixel(154.554668, 184.862948, 128.165682), FloatPixel(176.424991, 220.055567, 139.549646)},
                    {FloatPixel(155.520356, 196.498486, 152.874385), FloatPixel(173.566929, 223.443818, 153.306796)},
                    {FloatPixel(155.77202, 182.861839, 141.182531), FloatPixel(176.417361, 215.793141, 147.944463)}}}, // Basculin (Red Striped+Shiny, Blue Striped+Shiny, White Striped+Shiny)
            {585, {{FloatPixel(248.905624, 231.960547, 202.304008), /*Need shiny Spring Form form Deerling*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(200.765784, 221.649718, 157.010388), /*Need shiny Summer Form form Deerling*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(249.129347, 223.596864, 153.540712), /*Need shiny Autumn Form form Deerling*/ FloatPixel(255, 255, 255)},
                    {/*Need nonshiny Winter Form form Deerling*/ FloatPixel(255, 255, 255), /*Need shiny Winter Form form Deerling*/ FloatPixel(255, 255, 255)}}}, // Deerling (Spring+Shiny, Summer+Shiny, Autumn+Shiny, Winter+Shiny)
            {586, {{FloatPixel(226.703307, 210.355288, 183.036095), /*Need shiny Spring Form form Sawsbuck*/ FloatPixel(255, 255, 255)},
                    {/*Need nonshiny Summer Form form Sawsbuck*/ FloatPixel(255, 255, 255), /*Need shiny Summer Form form Sawsbuck*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(225.556512, 191.422413, 157.115032), /*Need shiny Autumn Form form Sawsbuck*/ FloatPixel(255, 255, 255)},
                    {FloatPixel(230.642283, 221.722029, 206.291687), /*Need shiny Winter Form form Sawsbuck*/ FloatPixel(255, 255, 255)}}}, // Sawsbuck (Spring+Shiny, Summer+Shiny, Autumn+Shiny, Winter+Shiny)
            {641, {{FloatPixel(180.638956, 187.422533, 160.762681), /*Need shiny Incarnate form Tornadus*/ FloatPixel(255, 255, 255)},
                    {/*Need nonshiny Therian form Tornadus*/ FloatPixel(255, 255, 255), /*Need shiny Therian form Tornadus*/ FloatPixel(255, 255, 255)}}}, // Tornadus (Incarnate+Shiny, Therian+Shiny)
            {642, {{FloatPixel(181.380142, 198.698165, 205.539588), /*Need shiny Incarnate form Thundurus*/ FloatPixel(255, 255, 255)},
                    {/*Need nonshiny Therian form Thundurus*/ FloatPixel(255, 255, 255), FloatPixel(217.579161, 205.614537, 235.753148)}}}, // Thundurus (Incarnate+Shiny, Therian+Shiny)
            {645, {{FloatPixel(216.922398, 184.366681, 141.085397), /*Need shiny Incarnate form Landorus*/ FloatPixel(255, 255, 255)},
                    {/*Need nonshiny Therian form Landorus*/ FloatPixel(255, 255, 255), /*Need shiny Therian form Landorus*/ FloatPixel(255, 255, 255)}}} // Landorus (Incarnate+Shiny, Therian+Shiny)
            // {647, {{}}} // Keldeo (Normal+Shiny, Resolute+Shiny)
            // {658, {{}}} // Greninja (Normal+Shiny, Ash Greninja+Shiny)
            // {666, {{}}} // Vivillon (Archipelago+Shiny, Continental+Shiny, Elegant+Shiny, Fancy+Shiny, Garden+Shiny, High Plains+Shiny, Icy Snow+Shiny, Jungle+Shiny, Marine+Shiny, Meadow+Shiny, Modern+Shiny, Monsoon+Shiny, Ocean+Shiny, Pokeball+Shiny, Polar+Shiny, River+Shiny, Sandstorm+Shiny, Savanna+Shiny, Sun+Shiny, Tundra+Shiny)
            // {669, {{}}} // Flabebe (Red+Shiny, Yellow+Shiny, Orange+Shiny, Blue+Shiny, White+Shiny)
            // {670, {{}}} // Floette (Red+Shiny, Yellow+Shiny, Orange+Shiny, Blue+Shiny, White+Shiny)
            // {671, {{}}} // Florges (Red+Shiny, Yellow+Shiny, Orange+Shiny, Blue+Shiny, White+Shiny)
            // {676, {{}}} // Furfrou (Normal+Shiny, Heart+Shiny, Star+Shiny, Diamond+Shiny, Debutante+Shiny, Matron+Shiny, Dandy+Shiny, La Reine+Shiny, Kabuki+Shiny, Pharoah+Shiny)
            // {710, {{}}} // Pumpkaboo (Average+Shiny, Small+Shiny, Large+Shiny, Super+Shiny)
            // {711, {{}}} // Gourgeist (Average+Shiny, Small+Shiny, Large+Shiny, Super+Shiny)
            // {720, {{}}} // Hoopa (Confined+Shiny, Unbound+Shiny)
            // {745, {{}}} // Lycanroc (Midday+Shiny, Midnight+Shiny, Dusk+Shiny)
            // {774, {{}}} // Minior
            // {801, {{}}} // Magearna
            // {849, {{}}} // Toxtricity
            // {893, {{}}} // Zarude
            // {901, {{}}} // Ursaluna
            // {905, {{}}} // Enamorus (Incarnate+Shiny, Therian+Shiny)
            // {925, {{}}} // Maushold
            // {931, {{}}} // Squawkabilly
            // {978, {{}}} // Tatsugiri
            // {982, {{}}} // Dudunsparce
            // {999, {{}}} // Gimmighoul
        };


        double temp = (size_t)this->national_dex_number;

        auto it = regional_codes.find(this->national_dex_number);
        if (!(it == regional_codes.end() || it->second.empty())) {
            for (size_t i = 0; i < it->second.size(); ++i) {
                if (this->type1 == it->second[i].first && this->type2 == it->second[i].second) {
                    temp += i * 0.01; // Encode form as a suffix
                }
            }
        }

        auto it2 = visual_forms.find(this->national_dex_number);
        if (!(it2 == visual_forms.end() || it2->second.empty())) {
            double min_distance = std::numeric_limits<double>::max();
            size_t form_id=0;

            size_t i =0;

            for (const auto& [nonshiny, shiny] : it2->second) {
                double nonshiny_distance = euclidean_distance(nonshiny, this->color);
                double shiny_distance = euclidean_distance(shiny, this->color);
                if (nonshiny_distance < min_distance) {
                    min_distance = nonshiny_distance;
                    form_id = i;
                }
                if (shiny_distance < min_distance) {
                    min_distance = shiny_distance;
                    form_id = i;
                }


                i++;
            }

            temp += form_id * 0.0001;
        }


        this->national_dex_number = temp;

    }


    // Log Pokemon details (to environment or as a string)
    std::string log_details(SingleSwitchProgramEnvironment* env = nullptr, bool log_to_env = false) const {
        std::ostringstream ss;
        ss << "Shiny: " << (shiny ? "Yes" : "No") << ", "
           << "Dex: " << national_dex_number << ", "
           << "Level: " << level << ", "
           << "Position: Box " << current_box
           << ", (" << current_row << ", " << current_col << ")\n";

        if (log_to_env && env) {
            env->console.log(ss.str());
        }
        return ss.str();
    }
};

class PokemonBox {
public:

    static constexpr size_t MAX_ROWS = 5;
    static constexpr size_t MAX_COLUMNS = 6;

    // Constructor to initialize a 6x5 grid.
    PokemonBox() : pokemon_count(0),blanks(MAX_ROWS * MAX_COLUMNS), consecutive_blanks(MAX_ROWS * MAX_COLUMNS), grid(MAX_ROWS, std::vector<std::shared_ptr<Pokemon>>(MAX_COLUMNS, nullptr)) {}

    // Function to add a Pokemon at a specific row and column.
    void add_pokemon(const std::shared_ptr<Pokemon>& pokemon, size_t row, size_t col) {
        if (row > MAX_ROWS || col > MAX_COLUMNS) {
            throw std::out_of_range("Row or column out of bounds.");
        }
        if (grid[row][col] != nullptr) {
            throw std::runtime_error("Position already occupied.");
        }
        pokemon->current_row = row;
        pokemon->current_col = col;
        grid[row][col] = pokemon;

        update_stats();
    }

    // Overloaded function to add a Pokemon based on its attributes.
    void populate_pokemon(const std::shared_ptr<Pokemon>& pokemon) {
        if (pokemon->current_row >= MAX_COLUMNS || pokemon->current_col >= MAX_ROWS) {
            throw std::out_of_range("Pokemon's current position is invalid.");
        }
        if (grid[pokemon->current_row][pokemon->current_col] != nullptr) {
            throw std::runtime_error("Position already occupied.");
        }
        grid[pokemon->current_row][pokemon->current_col] = pokemon;
        pokemon_count++;
        blanks--;
        consecutive_blanks--;
    }

    // Function to swap two Pokémon in the grid based on row and column positions.
    void swap_pokemon(size_t row1, size_t col1, size_t row2, size_t col2) {
        if (row1 > MAX_COLUMNS || col1 > MAX_ROWS || row2 > MAX_COLUMNS || col2 > MAX_ROWS) {
            throw std::out_of_range("Row or column out of bounds.");
        }

        // Swap the pointers (i.e., the Pokémon) between the two positions.
        std::swap(grid[row1][col1], grid[row2][col2]);

        // Update current positions of the Pokémon, if they are not nullptr.
        if (grid[row1][col1]) {
            grid[row1][col1]->current_row = row1;
            grid[row1][col1]->current_col = col1;
        }
        if (grid[row2][col2]) {
            grid[row2][col2]->current_row = row2;
            grid[row2][col2]->current_col = col2;
        }
    }

    void update_stats(){
        bool found_first = false;
        blanks = 0;
        for (size_t row = 0; row < MAX_ROWS; row++) {
            for (size_t col = 0; col < MAX_ROWS; col++) {
                if (grid[row][col] == nullptr) {
                    ++blanks;
                }else{
                    ++pokemon_count;
                    if(!found_first){
                        found_first = true;
                        first_poke_slot = {row, col};
                    }
                }
            }
        }

        consecutive_blanks = 0;
        for (int row = MAX_ROWS - 1; row >= 0; --row) {
            for (int col = MAX_COLUMNS - 1; col >= 0; --col) {
                if (grid[row][col] == nullptr) {
                    ++consecutive_blanks;
                } else {
                    return; // Stop when the first non-blank Pokémon is found
                }
            }
        }
    }


    // Print the current state of the box for debugging purposes.
    std::string print_box(SingleSwitchProgramEnvironment* env = nullptr, bool log_to_env = false) const {
        std::ostringstream ss;

        for (size_t row = 0; row < MAX_ROWS; ++row) {
            for (size_t col = 0; col < MAX_COLUMNS; ++col) {
                if (grid[row][col]) {
                    ss << "Pokémon at {" << row << ", " << col << "}\n" << grid[row][col]->log_details() << "\n";
                } else {
                    ss << "{" << row << ", " << col << "} Empty\n\n";
                }
            }
            ss << '\n';
        }
        if (log_to_env && env) {
            env->console.log(ss.str());
        }
        return ss.str();
    }

    size_t pokemon_count;
    size_t blanks;
    size_t consecutive_blanks;
    std::vector<std::vector<std::shared_ptr<Pokemon>>> grid;
    std::pair<size_t, size_t> first_poke_slot;
};

class BoxLayout {


public:
    BoxLayout() : layout(200, nullptr), sorted(200, false) {
    }

    ~BoxLayout() {
        // Clean up all allocated PokemonBox pointers
        for (size_t i = 0; i < layout.size(); ++i) {
            if (layout[i] != nullptr) {
                delete layout[i];
                layout[i] = nullptr;
            }
        }
    }

    void add_box(size_t index, PokemonBox* box) {
        if (index >= layout.size()) {
            throw std::out_of_range("Index out of bounds.");
        }
        layout[index] = box;
    }

    void update_box(size_t index, PokemonBox* new_box) {
        if (index >= layout.size()) {
            throw std::out_of_range("Index out of bounds.");
        }
        if (layout[index] == nullptr) {
            throw std::runtime_error("Cannot update an empty slot.");
        }
        layout[index] = new_box; // Replace the existing box with the new one.
    }

    PokemonBox* get_box(size_t index) const {
        if (index >= layout.size()) {
            throw std::out_of_range("Index out of bounds.");
        }
        return layout[index];
    }

    void set_sorted(size_t index, bool is_sorted) {
        if (index >= sorted.size()) {
            throw std::out_of_range("Index out of bounds.");
        }
        sorted[index] = is_sorted;
    }

    bool is_sorted(size_t index) const {
        if (index >= sorted.size()) {
            throw std::out_of_range("Index out of bounds.");
        }
        return sorted[index];
    }

    bool important_sorted(size_t start, size_t end) const {
        if (start > end || end >= sorted.size()) {
            throw std::out_of_range("Invalid range.");
        }
        for (size_t i = start; i <= end; ++i) {
            if (!sorted[i]) {
                return false;
            }
        }
        return true;
    }

private:

    std::vector<PokemonBox*> layout;
    std::vector<bool> sorted;
};


std::string sanitize_OCR(std::string str){
    char chars[] = "\n\r—.,";
    for(auto a:chars){str.erase(std::remove(str.begin(),str.end(), a),str.end());}
    return str;
}

std::string closest_type(FloatPixel& color_box){
    std::vector<std::pair<std::string,Color>> type_color_list = {{"Grass",Color(62, 180, 86)},{"Fire",Color(201, 106, 83)}, {"Water",Color(31, 161, 243)}, {"Electric",Color(202, 207, 66)}, {"Rock",Color(164, 201, 169)}, {"Ground",Color(145, 130, 78)}, {"Poison",Color(142, 125, 234)}, {"Dark",Color(86, 118, 113)}, {"Steel",Color(91, 188, 211)}, {"Flying",Color(112, 206, 242)}, {"Normal",Color(146, 190, 186)}, {"Fighting",Color(203, 173, 82)}, {"Ghost",Color(116, 125, 157)}, {"Dragon",Color(80, 145, 241)}, {"Ice",Color(44, 224, 243)}, {"Fairy",Color(202, 166, 242)}, {"Bug",Color(140, 188, 87)}, {"Psychic",Color(202, 128, 156)}, {"None",Color(20, 191, 195)}};

    double min_distance = std::numeric_limits<double>::max();
    std::string closest_type;

    for (const auto& [type, color] : type_color_list) {
        double distance = euclidean_distance(color_box, color);
        if (distance < min_distance) {
            min_distance = distance;
            closest_type = type;
        }
    }

    return closest_type;
}

std::shared_ptr<Pokemon> home_read_pokemon_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t box, size_t row, size_t col) {
    auto pokemon = std::make_shared<Pokemon>();



    pokemon->current_box = box;
    pokemon->current_row = row;
    pokemon->current_col = col;

    // Populate Pokémon attributes (same as before)
    ImageFloatBox national_dex_number_box(0.448, 0.245, 0.042, 0.04); //pokemon national dex number pos
    ImageFloatBox shiny_symbol_box(0.702, 0.09, 0.04, 0.06); // shiny symbol pos
    ImageFloatBox gmax_symbol_box(0.463, 0.09, 0.04, 0.06); // gmax symbol pos
    ImageFloatBox pokemon_box_small(0.76, 0.295, 0.14, 0.23); // pokemon render pos
    ImageFloatBox origin_symbol_box(0.623, 0.095, 0.033, 0.05); // origin symbol pos
    ImageFloatBox level_box(0.546, 0.099, 0.044, 0.041); // Level box
    ImageFloatBox type_1(0.622,0.245,0.029,0.053); // Get first type for pokemon
    ImageFloatBox type_2(0.654,0.245,0.029,0.053); // Get second type for pokemon

    VideoSnapshot screen = env.console.video().snapshot();
    VideoOverlaySet box_render(env.console);
    std::ostringstream ss;

    screen = env.console.video().snapshot();

    FloatPixel type_1_color = image_stats(extract_box_reference(screen, type_1)).average;
    FloatPixel type_2_color = image_stats(extract_box_reference(screen, type_2)).average;

    pokemon->type1 = closest_type(type_1_color);
    pokemon->type2 = closest_type(type_2_color);

    FloatPixel pokemon_color = image_stats(extract_box_reference(screen, pokemon_box_small)).average;
    pokemon->color = pokemon_color;

    int shiny_stddev_value = (int)image_stddev(extract_box_reference(screen, shiny_symbol_box)).sum();
    bool is_shiny = shiny_stddev_value > 30;
    pokemon->shiny = is_shiny;
    // env.console.log("Shiny detection stddev:" + std::to_string(shiny_stddev_value) + " is shiny:" + std::to_string(is_shiny));

    int gmax_stddev_value = (int)image_stddev(extract_box_reference(screen, gmax_symbol_box)).sum();
    bool is_gmax = gmax_stddev_value > 30;
    pokemon->gmax = is_gmax;
    // env.console.log("Gmax detection stddev:" + std::to_string(gmax_stddev_value) + " is gmax:" + std::to_string(is_gmax));

    int national_dex_number = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);
    if (national_dex_number < 0 || national_dex_number > 1025) {
        dump_image(env.console, ProgramInfo(), "ReadSummary_national_dex_number", screen);
    }
    pokemon->national_dex_number = (uint16_t)national_dex_number;
    pokemon->update_national_id();
    env.console.log(std::to_string(pokemon->national_dex_number));

    BoxGenderDetector::make_overlays(box_render);
    StatsHuntGenderFilter gender = BoxGenderDetector::detect(screen);
    // env.console.log("Gender: " + gender_to_string(gender), COLOR_GREEN);
    pokemon->gender = gender;

    // level_box
    int level = OCR::read_number_waterfill(env.console, extract_box_reference(screen, level_box), 0xff000000, 0xff7f7f7f);
    if (level < 0 || level > 100) {
        dump_image(env.console, ProgramInfo(), "ReadSummary_Level", screen);
    }
    pokemon->level = (uint16_t)level;
    // env.console.log("Level: " + std::to_string(level), COLOR_GREEN);


    return pokemon->national_dex_number>1025?home_read_pokemon_summary(env, context, box, row, col):pokemon;
}

std::pair<size_t,size_t> home_locate_home_position(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    int col;
    int row;
    FloatPixel temp[5][6];

    context.wait_for_all_requests();

    VideoSnapshot screen = env.console.video().snapshot();
    VideoOverlaySet box_render(env.console);

    for(int i = 0; i<5; i++){
        for(int j = 0; j<6; j++){
            ImageFloatBox slot_box(0.0735 + (0.072 * j), 0.165 + (0.1035 * i), 0.0055, 0.004);
            box_render.add(COLOR_RED, slot_box);
            FloatPixel current_box_value = image_stats(extract_box_reference(screen, slot_box)).average;
            // env.console.log(std::to_string(current_box_value.r)+" "+std::to_string(current_box_value.g)+" "+std::to_string(current_box_value.b));
            temp[i][j] = current_box_value;
        }
    }
    pbf_press_button(context, BUTTON_ZL, 10, 30);
    context.wait_for_all_requests();
    screen = env.console.video().snapshot();
    for(int i = 0; i<5; i++){
        for(int j = 0; j<6; j++){
            ImageFloatBox slot_box(0.0735 + (0.072 * j), 0.165 + (0.1035 * i), 0.0055, 0.004);
            box_render.add(COLOR_RED, slot_box);
            FloatPixel current_box_value = image_stats(extract_box_reference(screen, slot_box)).average;
            if(temp[i][j].r != current_box_value.r &&temp[i][j].g != current_box_value.g && temp[i][j].b != current_box_value.b){
                row = i;
                col = j;
            }
        }
    }
    pbf_press_button(context, BUTTON_ZR, 10, 30);
    context.wait_for_all_requests();
    box_render.clear();

    return {row,col};
}

//Move the cursor to the given coordinates, knowing current pos via the cursor struct
void move_cursor_to(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& current_cursor, const std::pair<size_t, size_t>& dest_cursor){

    std::ostringstream ss;
    ss << "Moving cursor from {" << current_cursor.first << ", " << current_cursor.second << "} to {" << dest_cursor.first << ", " << dest_cursor.second << "}";
    env.console.log(ss.str());

    std::pair<size_t, size_t> cur_cursor;
    if(current_cursor.first>5||current_cursor.second>6){
        size_t fails = 0;
        do{
            env.console.log("Found an invalid cursor position...reevaluating");
            if(++fails==6){
                fails = 0;
                pbf_press_button(context, BUTTON_B, 10, 50);
            }
            pbf_wait(context, 500ms);
            pbf_press_button(context, BUTTON_UP, 10, 50);
            pbf_press_button(context, BUTTON_UP, 10, 50);
            context.wait_for_all_requests();
            cur_cursor = home_locate_home_position(env, context);
        }while(cur_cursor.first>5||cur_cursor.second>6);
    }else{
        cur_cursor = current_cursor;
    }

    // direct nav up or down through rows
    if (!(cur_cursor.first == 0 && dest_cursor.first == 4) && !(dest_cursor.first == 0 && cur_cursor.first == 4)) {
        for (size_t i = cur_cursor.first; i < dest_cursor.first; ++i) {
            pbf_press_dpad(context, DPAD_DOWN, 10, 30);
        }
        for (size_t i = dest_cursor.first; i < cur_cursor.first; ++i) {
            pbf_press_dpad(context, DPAD_UP, 10, 30);
        }
    } else { // wrap around is faster to move between first or last row
        if (cur_cursor.first == 0 && dest_cursor.first == 4) {
            for (size_t i = 0; i <= 2; ++i) {
                pbf_press_dpad(context, DPAD_UP, 10, 30);
            }
        } else {
            for (size_t i = 0; i <= 2; ++i) {
                pbf_press_dpad(context, DPAD_DOWN, 10, 30);
            }
        }
    }

    // direct nav forward or backward through columns
    if ((dest_cursor.second > cur_cursor.second && dest_cursor.second - cur_cursor.second <= 3) || (cur_cursor.second > dest_cursor.second && cur_cursor.second - dest_cursor.second <= 3)) {
        for (size_t i = cur_cursor.second; i < dest_cursor.second; ++i) {
            pbf_press_dpad(context, DPAD_RIGHT, 10, 30);
        }
        for (size_t i = dest_cursor.second; i < cur_cursor.second; ++i) {
            pbf_press_dpad(context, DPAD_LEFT, 10, 30);
        }
    } else { // wrap around is faster if direct movement is more than 3 away
        if (dest_cursor.second > cur_cursor.second) {
            for (size_t i = 0; i < MAX_COLUMNS - (dest_cursor.second - cur_cursor.second); ++i) {
                pbf_press_dpad(context, DPAD_LEFT, 10, 30);
            }
        }
        if (cur_cursor.second > dest_cursor.second) {
            for (size_t i = 0; i < MAX_COLUMNS - (cur_cursor.second - dest_cursor.second); ++i) {
                pbf_press_dpad(context, DPAD_RIGHT, 10, 30);
            }
        }
    }

    context.wait_for_all_requests();
}

// Helps pick up the pokemon at the expected spot. Corrects for already holding a pokemon, not being in red selection mode, and cursor not being in the correct place.
void home_pick_up_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& cursor){

    VideoSnapshot screen = env.console.video().snapshot();

    ImageFloatBox slot_box(0.0735 + (0.072 * cursor.second), 0.165 + (0.1035 * cursor.first), 0.0055, 0.004);
    FloatPixel predicted_slot = image_stats(extract_box_reference(screen, slot_box)).average;

    bool b_pressed = false;
    int selection_modes = 0;
    while(predicted_slot.r <= predicted_slot.b+predicted_slot.g){ // Validate that expected spot is red

        if(selection_modes++==2){   // Scroll through 3 selection modes to find red
            pbf_press_button(context, BUTTON_ZR, 10, 35);
            if(!b_pressed){         // After 3 scrolls, assume we are holding a pokemon. Press b.
                b_pressed=true;
                pbf_press_button(context, BUTTON_B, 10, 35);
                selection_modes = 0;
            }else{                  // If we have already scrolled through selection modes, pressed b, and scrolled through again, the cursor must not be where we expect it. Find cursor.
                std::pair<size_t, size_t> temp_cursor = home_locate_home_position(env, context);
                move_cursor_to(env, context, temp_cursor, cursor);
                selection_modes = 0;
            }
        }

        context.wait_for_all_requests();

        screen = env.console.video().snapshot();
        predicted_slot = image_stats(extract_box_reference(screen, slot_box)).average;  // Update screen
    }

    FloatPixel predicted_slot2;

    do{
        pbf_press_button(context, BUTTON_Y, 10, 35);    // press y button

        // validate that expected spot is now white

        context.wait_for_all_requests();

        screen = env.console.video().snapshot();
        predicted_slot2 = image_stats(extract_box_reference(screen, slot_box)).average;  // Update screen

    }while(euclidean_distance(predicted_slot, predicted_slot2)==0);


}


// Need to build home_put_down_pokemon


void home_check_markings(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int col, int row){
    ImageFloatBox home_circle_marking(0.8075, 0.817, 0.0025, 0.005);
    ImageFloatBox home_triangle_marking(0.8375, 0.817, 0.0025, 0.005);
    ImageFloatBox home_square_marking(0.8725, 0.817, 0.0025, 0.005);
    ImageFloatBox home_heart_marking(0.9025, 0.817, 0.0025, 0.005);
    ImageFloatBox home_star_marking(0.935, 0.817, 0.0025, 0.005);
    ImageFloatBox home_diamond_marking(0.9675, 0.817, 0.0025, 0.005);

    ImageFloatBox home_circle_marking_big(0.75, 0.5, 0.0025, 0.005);
    ImageFloatBox home_triangle_marking_big(0.87, 0.5, 0.0025, 0.005);
    ImageFloatBox home_square_marking_big(0.75, 0.62, 0.0025, 0.005);
    ImageFloatBox home_heart_marking_big(0.87, 0.62, 0.0025, 0.005);
    ImageFloatBox home_star_marking_big(0.75, 0.75, 0.0025, 0.005);
    ImageFloatBox home_diamond_marking_big(0.87, 0.75, 0.0025, 0.005);

    context.wait_for_all_requests();
    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    box_render.add(COLOR_GREEN, home_circle_marking);
    box_render.add(COLOR_GREEN, home_triangle_marking);
    box_render.add(COLOR_GREEN, home_square_marking);
    box_render.add(COLOR_GREEN, home_heart_marking);
    box_render.add(COLOR_GREEN, home_star_marking);
    box_render.add(COLOR_GREEN, home_diamond_marking);



    std::vector<ImageFloatBox> marking_list = {home_circle_marking,home_triangle_marking,home_square_marking,home_heart_marking,home_star_marking,home_diamond_marking};
    bool marked = false;
    for(auto m:marking_list){
        FloatPixel marking = image_stats(extract_box_reference(screen, m)).average;

        marked = marked|(marking.r==255)|(marking.b==255);
    }

    if(marked){
        pbf_press_button(context, BUTTON_A, 10, 18);
        pbf_press_dpad(context, DPAD_DOWN,10, 40);
        pbf_press_dpad(context, DPAD_DOWN,10, 40);

        // Check can be marked in the first place
        context.wait_for_all_requests();
        screen = env.console.video().snapshot();
        ImageFloatBox can_mark(0.16 + (col * .0705), std::min(0.585, row*0.11+0.395), 0.0075, 0.01);
        box_render.add(COLOR_BLACK,can_mark);
        FloatPixel scan_val = image_stats(extract_box_reference(screen, can_mark)).average;
        env.console.log(std::to_string(scan_val.r)+" "+std::to_string(scan_val.g)+" "+std::to_string(scan_val.b));
        if(!(scan_val.r==255&&scan_val.b==0)){
            pbf_press_button(context, BUTTON_B, 10, 27);
            context.wait_for_all_requests();
            box_render.clear();
            return;
        }

        pbf_press_button(context, BUTTON_A, 10, 27);

        box_render.add(COLOR_GREEN, home_circle_marking_big);
        box_render.add(COLOR_GREEN, home_triangle_marking_big);
        box_render.add(COLOR_GREEN, home_square_marking_big);
        box_render.add(COLOR_GREEN, home_heart_marking_big);
        box_render.add(COLOR_GREEN, home_star_marking_big);
        box_render.add(COLOR_GREEN, home_diamond_marking_big);

        std::vector<ImageFloatBox> marking_list_big = {home_circle_marking_big,home_triangle_marking_big,home_square_marking_big,home_heart_marking_big,home_star_marking_big,home_diamond_marking_big};

        for(int i = 0; i<3; i++){
            for(int j=0;j<2;j++){
                while(image_stats(extract_box_reference(screen, marking_list_big[2*i+j])).average.r==255||image_stats(extract_box_reference(screen, marking_list_big[2*i+j])).average.b==255){
                    pbf_press_button(context, BUTTON_A, 10, 27);
                    context.wait_for_all_requests();
                    screen = env.console.video().snapshot();
                }
                pbf_press_dpad(context, DPAD_RIGHT, 10, 18);
            }
            pbf_press_dpad(context, DPAD_DOWN,10 ,18);
        }
        pbf_press_button(context, BUTTON_A, 10,27);

        box_render.clear();
    }else{
        env.console.log("No Markings Found");
    }

    context.wait_for_all_requests();

}

// This function is run in Home when loaded into a game. It is used to scroll left or right boxes to get to a target box
void home_navigate_to_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t target, bool hard_check = false){
    ImageFloatBox home_box_checker(0.075, 0.72, 0.03205, 0.04);
    VideoOverlaySet box_render(env.console);

    // Go to the first box in the program
    box_render.add(COLOR_RED, home_box_checker);

    size_t home_box;

    if(hard_check){
        env.console.log("Running hard check");
        context.wait_for_all_requests();
        VideoSnapshot screen = env.console.video().snapshot();
        home_box = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));

        if(home_box >= target){
            env.console.log("Initial read to right of target box "+std::to_string(target));
            size_t home_box2;
            size_t home_box3;
            do{
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                home_box = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));

                pbf_press_button(context, BUTTON_L, 10, 47);
                context.wait_for_all_requests();

                screen = env.console.video().snapshot();
                home_box2 = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));
                pbf_press_button(context, BUTTON_L, 10, 47);

                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                home_box3 = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));
                env.console.log("testing "+std::to_string(home_box2)+ "=="+std::to_string(home_box-1)+" && "+std::to_string(home_box3)+"=="+std::to_string(home_box-2));
            }while(!(home_box2==home_box-1&&home_box3==home_box-2));
            home_box-=2;
        }else{
            env.console.log("Initial read to left of target box "+std::to_string(target));
            size_t home_box2;
            size_t home_box3;
            do{
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                home_box = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));

                pbf_press_button(context, BUTTON_R, 10, 47);
                context.wait_for_all_requests();

                screen = env.console.video().snapshot();
                home_box2 = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));
                pbf_press_button(context, BUTTON_R, 10, 47);

                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                home_box3 = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));
                env.console.log("testing "+std::to_string(home_box2)+ "=="+std::to_string(home_box+1)+" && "+std::to_string(home_box3)+"=="+std::to_string(home_box+2));
            }while(!(home_box2==home_box+1&&home_box3==home_box+2));
            home_box+=2;
        }
    }else{
        context.wait_for_all_requests();
        VideoSnapshot screen = env.console.video().snapshot();
        home_box = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));
    }

    while(home_box!=target){
        while((home_box>200)){
            pbf_wait(context, 300);
            context.wait_for_all_requests();
            context.wait_for_all_requests();
            VideoSnapshot screen = env.console.video().snapshot();
            home_box = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));            }
        env.console.log("Home_box is " + std::to_string(home_box));
        context.wait_for_all_requests();
        box_render.clear();
        while(home_box<target){
            pbf_press_button(context, BUTTON_R, 10, 47);
            home_box++;
        }
        while(home_box>target){
            pbf_press_button(context, BUTTON_L, 10, 47);
            home_box--;
        }
        context.wait_for_all_requests();
        box_render.add(COLOR_RED, home_box_checker);
        context.wait_for_all_requests();
        VideoSnapshot screen = env.console.video().snapshot();
        home_box = std::stoull(sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_box_checker))));
    }
}

void home_navigate_to_box_secondary(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string target){
    ImageFloatBox home_box_checker_secondary(0.62, 0.105, 0.255, 0.04);
    VideoOverlaySet box_render(env.console);

    // Go to the first box in the program
    box_render.add(COLOR_RED, home_box_checker_secondary);

    box_render.add(COLOR_GREEN, home_box_checker_secondary);
    context.wait_for_all_requests();
    std::string box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), home_box_checker_secondary)));
    while(target!=box_name){   // navigate to correct box
        pbf_press_button(context, BUTTON_L, 10, 80);
        context.wait_for_all_requests();
        box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), home_box_checker_secondary)));
    }
}

void home_navigate_to_box_secondary(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t target){
    ImageFloatBox home_box_checker_secondary(0.85, 0.725, 0.03, 0.03);
    VideoOverlaySet box_render(env.console);

    // Go to the first box in the program
    box_render.add(COLOR_RED, home_box_checker_secondary);

    size_t home_box = OCR::read_number_waterfill(env.console, extract_box_reference(env.console.video().snapshot(), home_box_checker_secondary), 0xff000000, 0xff7f7f7f);

    while(home_box!=target){
        while((home_box>200)){
            pbf_wait(context, 300);
            context.wait_for_all_requests();
            home_box = OCR::read_number_waterfill(env.console, extract_box_reference(env.console.video().snapshot(), home_box_checker_secondary), 0xff000000, 0xff7f7f7f);
        }
        env.console.log("Home_box is " + std::to_string(home_box));
        context.wait_for_all_requests();
        box_render.clear();
        while(home_box<target){
            pbf_press_button(context, BUTTON_R, 10, 47);
            home_box++;
        }
        while(home_box>target){
            pbf_press_button(context, BUTTON_L, 10, 47);
            home_box--;
        }
        context.wait_for_all_requests();
        box_render.add(COLOR_RED, home_box_checker_secondary);
        home_box = OCR::read_number_waterfill(env.console, extract_box_reference(env.console.video().snapshot(), home_box_checker_secondary), 0xff000000, 0xff7f7f7f);
    }

    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    box_render.clear();

}

void home_navigate_to_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Game& game){
    ImageFloatBox game_checker(0.0455, 0.244, 0.435, 0.057);
    std::string text = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), game_checker)));
    VideoOverlaySet box_render(env.console);
    box_render.add(COLOR_BLUE, game_checker);

    do{
        env.console.log("Found game " + text + OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), game_checker)));

        if(text==" POKEMON "){
            pbf_press_button(context, BUTTON_A, 10, 400);
        }else{
            pbf_press_dpad(context, DPAD_RIGHT, 10, 40);
        }
        context.wait_for_all_requests();
        text = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), game_checker)));
    }while(text!=game.name);
    pbf_press_button(context, BUTTON_A, 10, 100);
    pbf_press_button(context, BUTTON_A, 10, 150*15);

}

bool home_exit_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ImageFloatBox save_msg_checker(0.155, 0.82, 0.35, 0.06);
    VideoOverlaySet box_render(env.console);

    pbf_press_button(context, BUTTON_PLUS, 10, 47);
    pbf_wait(context, 100);
    pbf_press_button(context, BUTTON_A, 10, 47);
    std::string text;
    int checks = 0;
    pbf_wait(context, 5000ms);
    do{
        pbf_wait(context, 1000ms);
        box_render.add(COLOR_BLUE, save_msg_checker);
        context.wait_for_all_requests();
        text = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), save_msg_checker)));
        if(checks++ == 200){
            return false;
        }
    }while(text!="Your Boxes have been saved!");
    pbf_press_button(context, BUTTON_A, 10, 47);
    return true;
}

void home_scan_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, VideoSnapshot screen, bool clear_markings=false, bool release_go=false){
    move_cursor_to(env, context, home_locate_home_position(env, context), {0,0});

    for(int i = 0; i<5;i++){
        for(int j = 0; j<6; j++){
            screen = env.console.video().snapshot();
            if(clear_markings)home_check_markings(env, context, j, i);
            pbf_press_dpad(context, DPAD_RIGHT, 10, 27);
        }
        pbf_press_dpad(context, DPAD_DOWN, 10, 27);
        context.wait_for_all_requests();
    }
    pbf_press_dpad(context, DPAD_DOWN, 10, 27);
    pbf_press_dpad(context, DPAD_DOWN, 10, 27);

    context.wait_for_all_requests();
}

std::pair<size_t,size_t> home_locate_empty_position(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t* current_box, size_t last_box){
    VideoOverlaySet box_render(env.console);

    VideoSnapshot screen = env.console.video().snapshot();

    while(*current_box<last_box){
        for (size_t row = 0; row < 5; row++){
            for (size_t column = 0; column < 6; column++){
                ImageFloatBox slot_box(0.06 + (0.072 * column), 0.2 + (0.1035 * row), 0.03, 0.057);
                int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

                //checking color to know if a pokemon is on the slot or not
                if(current_box_value < 5){
                    return {row, column};
                }
            }
        }
        pbf_press_button(context, BUTTON_R, 10, 80);
        (*current_box)++;
        context.wait_for_all_requests();
        screen = env.console.video().snapshot();
    }
    throw;
}

std::pair<size_t,size_t> home_locate_empty_position_secondary(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t* current_box, size_t last_box, bool transpose=false){
    VideoOverlaySet box_render(env.console);

    VideoSnapshot screen = env.console.video().snapshot();

    if(transpose){
        while(*current_box<last_box){
            for (size_t column = 0; column < 6; column++){
                for (size_t row = 0; row < 5; row++){
                    ImageFloatBox slot_box(0.55 + (0.072 * column), 0.2 + (0.1035 * row), 0.03, 0.057);
                    int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

                    //checking color to know if a pokemon is on the slot or not
                    if(current_box_value < 5){
                        return {row, column};
                    }
                }
            }
            pbf_press_button(context, BUTTON_R, 10, 80);
            (*current_box)++;
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
        }
    }else{
        while(*current_box<last_box){
            for (size_t row = 0; row < 5; row++){
                for (size_t column = 0; column < 6; column++){
                    ImageFloatBox slot_box(0.55 + (0.072 * column), 0.2 + (0.1035 * row), 0.03, 0.057);
                    int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

                    //checking color to know if a pokemon is on the slot or not
                    if(current_box_value < 5){
                        return {row, column};
                    }
                }
            }
            pbf_press_button(context, BUTTON_R, 10, 80);
            (*current_box)++;
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
        }
    }
    throw;
}

void home_swap_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& slot1, const std::pair<size_t, size_t>& slot2){
    move_cursor_to(env, context, home_locate_home_position(env, context), slot1);

    home_pick_up_pokemon(env, context, slot1);

    move_cursor_to(env, context, slot1, slot2);

    pbf_press_button(context, BUTTON_Y, 10, 80);

    context.wait_for_all_requests();
}

void home_swap_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::pair<size_t, size_t>& slot1, const std::pair<size_t, size_t>& slot2, const std::pair<size_t, size_t>& curr){
    move_cursor_to(env, context, curr, slot1);

    home_pick_up_pokemon(env, context, slot1);

    move_cursor_to(env, context, slot1, slot2);

    pbf_press_button(context, BUTTON_Y, 10, 80);

    context.wait_for_all_requests();
}

bool home_request_next(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Game& game){
    ImageFloatBox home_filter_reader(0.4, 0.41, 0.2, 0.045);

    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);


    pbf_press_button(context, BUTTON_X, 10, 80);
    pbf_press_button(context, BUTTON_Y, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_X, 10, 30);
    pbf_press_button(context, BUTTON_UP, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 30);
    pbf_press_button(context, BUTTON_UP, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 30);

    size_t temp = game.index;
    env.console.log(std::to_string(temp));
    do{
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        env.console.log(std::to_string(temp));
    }while(--temp==0);
    pbf_press_button(context, BUTTON_A, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 35);
    pbf_press_button(context, BUTTON_A, 10, 35);
    pbf_press_button(context, BUTTON_UP, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 30);
    pbf_press_button(context, BUTTON_B, 10, 30);

    pbf_wait(context, 1500ms);
    context.wait_for_all_requests();
    screen = env.console.video().snapshot();
    box_render.add(COLOR_GREEN, home_filter_reader);
    std::string filter_text = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, home_filter_reader)));
    env.console.log(filter_text);

    if(filter_text=="No matches found!")return false;


    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);

    context.wait_for_all_requests();

    return true;
}

// This function is run in Pokémon Home, and should be used when wanting to transfer a Pokémon from the Home box into the Game box on the right.
// Requires env, context, and two coordinates sent as an std::pair<size_t,size_t>
void home_move_pokemon_to_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::pair<size_t,size_t> home, std::pair<size_t,size_t> game){
    home_pick_up_pokemon(env, context, home);

    if(home.first==0&&game.first==5){
        for(int i = 0; i<3; i++){
            pbf_press_button(context, BUTTON_UP, 10, 30);
        }
    }else{
        if(home.first>game.first){ // moving up
            for(size_t i = home.first-game.first; i>0; i--){
                pbf_press_button(context, BUTTON_UP, 10, 30);
            }
        }else if(home.first<game.first){ // moving down
            for(size_t i = game.first-home.first; i>0; i--){
                pbf_press_button(context, BUTTON_DOWN, 10, 30);
            }
        }
    }
    if(game.second<=home.second){ // scrolling right
        for(size_t i = 0; i < 6 + game.second - home.second; i++){
            pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        }
    }else{ // wrap-around, scrolling left
        for(size_t i = 0; i < 6 + home.second - game.second; i++){
            pbf_press_button(context, BUTTON_LEFT, 10, 35);
        }
    }

    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_UP, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);

    context.wait_for_all_requests();

}

// This function is run in Pokémon Home, and should be used when wanting to transfer a Pokémon to the Home box from the Game box on the right.
// Assumes the cursor is in Home position {0,5}
// Requires env, context, and two coordinates sent as an std::pair<size_t,size_t>
void home_move_pokemon_from_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::pair<size_t,size_t> home, std::pair<size_t,size_t> game, bool emergency = false){
    // move to the non-home box first
    if(game.first== 5){
        for(size_t i = 0; i<3; i++){
            pbf_press_button(context, BUTTON_UP, 10, 30);
        }
    }else{
        for(size_t i = 0; i<game.first; i++){
            pbf_press_button(context, BUTTON_DOWN, 10, 30);
        }
    }
    for(size_t i = 0; i < game.second + 1; i++){
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
    }

    home_pick_up_pokemon(env, context, game);    //place it in the home box
    if(game.first==0&&home.first==5){
        for(size_t i = 0; i<3; i++){
            pbf_press_button(context, BUTTON_UP, 10, 30);
        }
    }else{
        if(game.first>home.first){ // moving up
            for(size_t i = game.first-home.first; i>0; i--){
                pbf_press_button(context, BUTTON_UP, 10, 30);
            }
        }else if(game.first<home.first){ // moving down
            for(size_t i = home.first-game.first; i>0; i--){
                pbf_press_button(context, BUTTON_DOWN, 10, 30);
            }
        }
    }
    if(home.second<=game.second){ // scrolling right
        for(size_t i = 0; i < 6 + home.second - game.second; i++){
            pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        }
    }else{ // wrap-around, scrolling left
        for(size_t i = 0; i < 6 + game.second - home.second; i++){
            pbf_press_button(context, BUTTON_LEFT, 10, 35);
        }
    }

    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);
    if(emergency)pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_UP, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, 50);

    // Move back to {0,5}
    if(home.first==4){
        for(size_t i = 0; i<3; i++){
            pbf_press_button(context, BUTTON_DOWN, 10, 30);
        }
    }else{
        for(size_t i = 0; i<home.first; i++){
            pbf_press_button(context, BUTTON_UP, 10, 30);
        }
    }
    for(size_t i = home.second; i < 5; i++){
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
    }

    context.wait_for_all_requests();

}

// This function is run in Pokémon Home, and should be used when wanting to locate all Pokémon eligible for powering up
// Requires env, context, and a game name to identify what game to navigate to
// Returns the amount of Pokémon successfully retrieved
int home_fill_boxes_to_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Game& game, std::string box_name){
    pbf_press_button(context, BUTTON_LEFT, 10, 30);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    home_navigate_to_box_secondary(env, context, box_name);
    int found = 0;
    for(int i =0; i<6; i++){
        for(int j = 0; j < 5; j++){
            if(!home_request_next(env, context, game)){
                pbf_press_button(context, BUTTON_B, 10, 150);
                home_exit_home(env, context);
                return found;
            }
            pbf_wait(context, 500ms);
            context.wait_for_all_requests();
            std::pair<size_t, size_t> home_pos = home_locate_home_position(env, context);

            // env.console.log(std::to_string(home_pos.first)+" "+std::to_string(home_pos.second));
            // env.console.log(std::to_string(i)+" "+std::to_string(j));
            home_move_pokemon_to_game(env, context, home_pos, {j,i});
            found++;
        }
    }

    home_exit_home(env, context);
    return found;
}

PokemonBox home_build_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::pair<size_t,size_t>& cursor, size_t box_num = 0){
    context.wait_for_all_requests();

    VideoOverlaySet box_render(env.console);

    VideoSnapshot screen = env.console.video().snapshot();

    PokemonBox tempbox;

    std::vector<std::pair<size_t,size_t>> blank_list;
    std::pair<size_t, size_t> first_poke_slot = {0, 0};
    bool find_first_poke = false;
    size_t pokemon_count = 0;

    // std::pair<size_t,size_t> cursor=home_locate_home_position(env, context);

    move_cursor_to(env, context, cursor, {0,0});
    cursor = {0,0};

    for (size_t row = 0; row < 5; row++){
        for (size_t column = 0; column < 6; column++){
            ImageFloatBox slot_box(0.06 + (0.072 * column), 0.2 + (0.1035 * row), 0.03, 0.057);
            int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

            //checking color to know if a pokemon is on the slot or not
            if(current_box_value < 5){
                box_render.add(COLOR_RED, slot_box);
                blank_list.push_back({row, column});
            }else{
                box_render.add(COLOR_GREEN, slot_box);
                pokemon_count++;
                if(find_first_poke == false){
                    first_poke_slot = {row, column};
                    find_first_poke = true;
                }
            }
        }
    }

    if(!find_first_poke){
        tempbox.update_stats();
        return tempbox;
    }

    move_cursor_to(env, context, {0,0}, first_poke_slot);
    cursor = first_poke_slot;

    box_render.clear();

    pbf_press_button(context, BUTTON_A, 10, 50);
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 180);
    context.wait_for_all_requests();


    size_t blanks = 0;
    for(size_t i = 0; i < pokemon_count; i++){
        size_t row = (size_t)(i+blanks)/6;
        size_t col = (i+blanks)%6;
        std::pair<size_t,size_t> temp = {row, col};
        if (!blank_list.empty() && blank_list[0] == temp) {
            i--;
            blanks++;
            blank_list.erase(blank_list.begin());
        }else{
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
            std::shared_ptr<Pokemon> temp_pokemon(home_read_pokemon_summary(env, context, box_num, row, col));
            tempbox.add_pokemon(temp_pokemon, row, col);
            pbf_press_button(context, BUTTON_R, 10, 65);
        }
    }

    tempbox.update_stats();
    pbf_press_button(context, BUTTON_B, 10, 270);
    context.wait_for_all_requests();

    tempbox.print_box(&env, true);

    return tempbox;
}

bool home_sort_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox& box, std::pair<size_t,size_t>& cursor) {
    bool touched = false;
    size_t rows = MAX_ROWS;
    size_t cols = MAX_COLUMNS;

    bool reverse = false;

    // Perform a selection sort on the grid
    for (size_t i = 0; i < rows * cols - 1; ++i) {
        size_t min_idx = i;
        size_t min_row = i / cols;
        size_t min_col = i % cols;

        // Find the "smallest" Pokémon (by national dex, level, etc.)
        for (size_t j = i; j < rows * cols; ++j) {
            size_t row = j / cols;
            size_t col = j % cols;

            // Comparison logic:
            if (!box.grid[min_row][min_col] ||
                (box.grid[row][col] &&
                 (!box.grid[min_row][min_col] || *box.grid[row][col] < *box.grid[min_row][min_col]))) {
                min_idx = j;
                min_row = row;
                min_col = col;
            }

        }

        // If the min_idx is different from the current index, swap them
        if (min_idx != i) {
            touched = true;
            size_t swap_row = i / cols;
            size_t swap_col = i % cols;

            // Define slots for swapping
            std::pair<size_t, size_t> slot1 = {swap_row, swap_col};
            std::pair<size_t, size_t> slot2 = {min_row, min_col};

            if(box.grid[slot1.first][slot1.second] == nullptr && box.grid[slot2.first][slot2.second] == nullptr)continue;


            // set up easy swapping, given that there are no blank spaces
            if(reverse && box.grid[slot2.first][slot2.second] == nullptr){
                reverse = false;
            }else if (!reverse && box.grid[slot1.first][slot1.second] == nullptr){
                reverse = true;

            }

            // For time saving, if the swap spot is next, don't reverse (don't trigger if current spot is blank, uncommon)
            if(reverse && i + 1 == min_idx && box.grid[slot1.first][slot1.second] != nullptr){
                reverse = false;
            }

            // I think this is where the consecutive blanks logic goes if at all (in an else block, separating out the nullptr check?)

            // Call home_swap_pokemon using last_position if available
            if (reverse) {
                home_swap_pokemon(env, context, slot2, slot1, cursor);
            } else {
                home_swap_pokemon(env, context, slot1, slot2, cursor);
            }

            // Update the last position to slot2 after the swap
            cursor = reverse ? slot1 : slot2;

            reverse = !reverse;


            // Swap in the box
            box.swap_pokemon(swap_row, swap_col, min_row, min_col);
        }
    }

    box.update_stats();

    context.wait_for_all_requests();
    return touched;
}

size_t home_dirty_swap_window_size(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox left, PokemonBox right){
    size_t window_size = 0;
    for(size_t i = 0; i < 30; i++){
        for(size_t j = 0; j <= i; j++){
            if(left.grid[(size_t)(29-i+j)/6][(29-i+j)%6]==nullptr)continue;
            if(right.grid[(size_t)(0+j)/6][(0+j)%6]==nullptr)return window_size;
            if(*(left.grid[(size_t)(29-i+j)/6][(29-i+j)%6])<*(right.grid[(size_t)(0+j)/6][(0+j)%6])){
                return window_size;
            }else{
                env.console.log(left.grid[(size_t)(29-i+j)/6][(29-i+j)%6]->log_details(&env));
                env.console.log(right.grid[(size_t)(0+j)/6][(0+j)%6]->log_details(&env));
            }
        }
        window_size++;
    }
    return window_size;
}

size_t home_dirty_swap_window_size_v2(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox left, PokemonBox right){
    size_t window_size = 0;
    for(size_t i = 0; i < 30; i++){
        for(size_t j = 0; j <=i; j++){
            if(left.grid[(size_t)(29-i+j)/6][(29-i+j)%6]==nullptr)continue;
            if(right.grid[(size_t)(0+j)/6][(0+j)%6]==nullptr)return window_size;
            if(*(left.grid[(size_t)(29-i)/6][(29-i)%6])<=*(right.grid[(size_t)(0+j)/6][(0+j)%6])){
                return window_size;
            }else{
                env.console.log(left.grid[(size_t)(29-i+j)/6][(29-i+j)%6]->log_details(&env));
                env.console.log(right.grid[(size_t)(0+j)/6][(0+j)%6]->log_details(&env));
            }
        }
        window_size++;
    }
    return window_size;
}

bool home_sort_box_v2(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox& left, PokemonBox& right, bool sort_left) {
    bool touched = false;
    size_t rows = MAX_ROWS;
    size_t cols = MAX_COLUMNS;

    bool reverse = false;

    // Optional to track the last cursor position
    std::optional<std::pair<size_t, size_t>> last_position;

    PokemonBox& box= sort_left?left:right;

    PokemonBox temp = sort_left?right:left;

    // Step 1: Flatten the grid into a single vector of shared pointers to Pokémon
    std::vector<std::shared_ptr<Pokemon>> allPokemons;
    for (auto& row : temp.grid) {
        allPokemons.insert(allPokemons.end(), row.begin(), row.end());
    }

    // Step 2: Sort the vector of Pokémon
    std::sort(allPokemons.begin(), allPokemons.end(), [](const std::shared_ptr<Pokemon>& a, const std::shared_ptr<Pokemon>& b) {
        return *a < *b; // Sorting by Pokémon object, assuming operator< is defined for Pokémon
    });

    // Step 3: Rebuild the grid with the sorted Pokémon
    auto it = allPokemons.begin();
    for (auto& row : temp.grid) {
        for (auto& pokemon : row) {
            pokemon = *it; // Assign the sorted Pokémon back to the grid
            ++it;
        }
    }

    temp.print_box(&env);

    size_t final_window_size = home_dirty_swap_window_size_v2(env, context, sort_left?temp:left, sort_left?right:temp);
    env.console.log("final window size is "+std::to_string(final_window_size));
    if(final_window_size==0||final_window_size==30){
        env.console.log("quitting early");
        return true;
    }


    env.console.log("In the loop");

    // Perform a selection sort on the grid
    for (size_t i = 0; i < rows * cols - 1; ++i) {
        size_t min_idx = i;
        size_t min_row = i / cols;
        size_t min_col = i % cols;

        // Find the "smallest" Pokémon (by national dex, level, etc.)
        for (size_t j = i; j < rows * cols; ++j) {
            size_t row = j / cols;
            size_t col = j % cols;

            // Comparison logic:
            if (!box.grid[min_row][min_col] ||
                (box.grid[row][col] &&
                 (!box.grid[min_row][min_col] || *box.grid[row][col] < *box.grid[min_row][min_col]))) {
                min_idx = j;
                min_row = row;
                min_col = col;
            }

        }

        // If the min_idx is different from the current index, swap them
        if (min_idx != i) {
            touched = true;
            env.console.log("Starting swap");
            size_t swap_row = i / cols;
            size_t swap_col = i % cols;

            // Define slots for swapping
            std::pair<size_t, size_t> slot1 = {swap_row, swap_col};
            std::pair<size_t, size_t> slot2 = {min_row, min_col};

            if(box.grid[slot1.first][slot1.second] == nullptr && box.grid[slot2.first][slot2.second] == nullptr)continue;

            env.console.log("Swapping in home {" + std::to_string(slot1.first) + ", " + std::to_string(slot1.second) + "} and {" + std::to_string(slot2.first) + ", " + std::to_string(slot2.second) + "}");

            // set up easy swapping, given that there are no blank spaces
            if(reverse && box.grid[slot2.first][slot2.second] == nullptr){
                reverse = false;
            }else if (!reverse && box.grid[slot1.first][slot1.second] == nullptr){
                reverse = true;

            }

            // For time saving, if the swap spot is next, don't reverse (don't trigger if current spot is blank, uncommon)
            if(reverse && i + 1 == min_idx && box.grid[slot1.first][slot1.second] != nullptr){
                reverse = false;
            }

            // I think this is where the consecutive blanks logic goes if at all (in an else block, separating out the nullptr check?)

            // Call home_swap_pokemon using last_position if available
            if (last_position) {
                if (reverse) {
                    home_swap_pokemon(env, context, slot2, slot1, *last_position);
                } else {
                    home_swap_pokemon(env, context, slot1, slot2, *last_position);
                }
            } else {
                if (reverse) {
                    home_swap_pokemon(env, context, slot2, slot1);
                } else {
                    home_swap_pokemon(env, context, slot1, slot2);
                }
            }

            // Update the last position to slot2 after the swap
            last_position = reverse ? slot1 : slot2;

            reverse = !reverse;

            env.console.log("Swapping in box");

            // Swap in the box
            box.swap_pokemon(swap_row, swap_col, min_row, min_col);

            if(home_dirty_swap_window_size_v2(env, context, sort_left?box:left, sort_left?right:box)){
                env.console.log("quitting early");
                return true;
            }
        }
    }

    // After sorting, loop through the box to count consecutive blanks from the end
    size_t consecutive_blanks = 0;
    for (size_t row = rows - 1; row < rows; --row) {
        for (size_t col = cols - 1; col < cols; --col) {
            if (box.grid[row][col] == nullptr) {
                ++consecutive_blanks;
            } else {
                break; // Stop if we encounter a non-blank Pokémon
            }
        }
    }

    if(touched)move_cursor_to(env, context, *last_position, {4,5});
    context.wait_for_all_requests();
    box.consecutive_blanks = consecutive_blanks;
    return touched;
}

bool home_do_dirty_swap_v2(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox& left, PokemonBox& right, std::pair<size_t, size_t> cursor = {4, 5}) {
    size_t window = home_dirty_swap_window_size_v2(env, context, left, right);
    context.wait_for_all_requests();
    std::pair<size_t, size_t> last_position = cursor;

    bool right_box = true;

    for(size_t i = 0; i < window; i++){
        size_t left_index = 30 - window + i;
        size_t right_index = 0 + i;

        size_t left_row = left_index / 6;
        size_t left_col = left_index % 6;
        size_t right_row = right_index / 6;
        size_t right_col = right_index % 6;

        env.console.log(std::to_string(left.grid[left_row][left_col] != nullptr));
        env.console.log(std::to_string(i%2==1));
        env.console.log(std::to_string(!(left.grid[left_row][left_col] != nullptr) != !(i%2==1)));

        // set up easy swapping, given that there are no blank spaces
        if(right_box && right.grid[right_row][right_col] == nullptr){
            pbf_press_button(context, BUTTON_L, 10, 150);
            right_box = false;
        }else if (!right_box && left.grid[left_row][left_col] == nullptr){
            pbf_press_button(context, BUTTON_R, 10, 150);
            right_box = true;
        }else if (right.grid[right_row][right_col] == nullptr && left.grid[left_row][left_col] == nullptr)continue;

        // Do easy swapping, knowing what box we are starting from
        if(right_box){
            move_cursor_to(env, context, last_position, {right_row, right_col});
            home_pick_up_pokemon(env, context, {right_row, right_col});
            pbf_press_button(context, BUTTON_L, 10, 100);
            move_cursor_to(env, context, {right_row, right_col}, {left_row, left_col});
            pbf_press_button(context, BUTTON_Y, 10, 70);
            last_position = {left_row, left_col};
            right_box = false;
        } else {
            move_cursor_to(env, context, last_position, {left_row, left_col});
            home_pick_up_pokemon(env, context, {left_row, left_col});
            pbf_press_button(context, BUTTON_R, 10, 100);
            move_cursor_to(env, context, {left_row, left_col}, {right_row, right_col});
            pbf_press_button(context, BUTTON_Y, 10, 70);
            last_position = {right_row, right_col};
            right_box = true;
        }

        std::swap(left.grid[left_row][left_col], right.grid[right_row][right_col]);

    }

    // Return to right box
    if(!right_box){
        pbf_press_button(context, BUTTON_R, 10, 100);
    }

    return window>0;
}

bool home_dirty_swap_check(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox left, PokemonBox right, size_t diff = 0){
    // Change this function to calculate sliding window size instead of spread away from middle so that stuff isn't inverted randomly

    if(!left.grid[(size_t)(29-diff)/6][(29-diff)%6]) return true;
    if(!right.grid[(size_t)(0+diff)/6][(0+diff)%6]) return false;
    env.console.log("At box " + std::to_string(left.grid[(size_t)(29-diff)/6][(29-diff)%6]->current_box) + " ("+std::to_string((size_t)(0+diff)/6)+", "+std::to_string((0+diff)%6)+")");
    env.console.log("At box " + std::to_string(right.grid[(size_t)(0+diff)/6][(0+diff)%6]->current_box) + " ("+std::to_string((size_t)(29-diff)/6)+", "+std::to_string((29-diff)%6)+")");
    env.console.log(std::to_string(*(right.grid[(size_t)(0+diff)/6][(0+diff)%6]) < *(left.grid[(size_t)(29-diff)/6][(29-diff)%6]))+left.grid[(size_t)(29-diff)/6][(29-diff)%6]->log_details()+right.grid[(size_t)(0+diff)/6][(0+diff)%6]->log_details());
    if(*(right.grid[(size_t)(0+diff)/6][(0+diff)%6])<*(left.grid[(size_t)(29-diff)/6][(29-diff)%6])) return true;
    return false;
}

bool home_do_dirty_swap(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox& left, PokemonBox& right, std::pair<size_t, size_t> cursor = {4, 5}) {
    bool ret = home_dirty_swap_check(env, context, left, right);
    context.wait_for_all_requests();
    size_t iterations = 0;

    std::pair<size_t, size_t> last_position = cursor;

    if (ret) {
        do {
            size_t left_index = 29 - iterations;
            size_t right_index = 0 + iterations;

            size_t left_row = left_index / 6;
            size_t left_col = left_index % 6;
            size_t right_row = right_index / 6;
            size_t right_col = right_index % 6;

            if (left.grid[left_row][left_col] != nullptr) {
                pbf_press_button(context, BUTTON_L, 10, 150);
                move_cursor_to(env, context, last_position, {left_row, left_col});
                home_pick_up_pokemon(env, context, {left_row, left_col});
                pbf_press_button(context, BUTTON_R, 10, 100);
                move_cursor_to(env, context, {left_row, left_col}, {right_row, right_col});
                pbf_press_button(context, BUTTON_Y, 10, 70);
                last_position = {right_row, right_col};
            } else {
                move_cursor_to(env, context, last_position, {right_row, right_col});
                home_pick_up_pokemon(env, context, {right_row, right_col});
                pbf_press_button(context, BUTTON_L, 10, 100);
                move_cursor_to(env, context, {right_row, right_col}, {left_row, left_col});
                pbf_press_button(context, BUTTON_Y, 10, 70);
                pbf_press_button(context, BUTTON_R, 10, 100);
                last_position = {left_row, left_col};
            }

            // Perform the swap in the actual grid
            std::swap(left.grid[left_row][left_col], right.grid[right_row][right_col]);


        } while (++iterations < 29 && home_dirty_swap_check(env, context, left, right, iterations));
    }

    context.wait_for_all_requests();
    return ret;
}

bool home_reconcile_blanks(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox& box){
    VideoSnapshot screen = env.console.video().snapshot();

    // size_t blanks = 0;

    for (size_t row = 0; row < 5; row++){
        for (size_t column = 0; column < 6; column++){
            ImageFloatBox slot_box(0.06 + (0.072 * column), 0.2 + (0.1035 * row), 0.03, 0.057);
            int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

            if(box.grid[row][column]!=nullptr != current_box_value >= 5){ // nullptr xor blank space
                // blanks++;
                // env.console.log("Found misaligned blank at {"+std::to_string(row)+", "+std::to_string(column)+"}");
                return false;
            }else{
                // env.console.log("Spot was validated");
            }
        }
    }
    // if(blanks==0)env.console.log("No blanks detected.");
    return true;
}

bool home_make_easy_swaps(SingleSwitchProgramEnvironment& env, ProControllerContext& context, PokemonBox& left, PokemonBox& right, std::pair<size_t, size_t>& cursor, bool leftBox = true){

    env.console.log("Left blanks = "+std::to_string(left.blanks));
    env.console.log("Right blanks = "+std::to_string(right.blanks));


    if(left.blanks==30){
        if(leftBox){
            pbf_press_button(context, BUTTON_R, 10, 35);
        }
        move_cursor_to(env, context, cursor, {0,0});
        pbf_press_button(context, BUTTON_ZR, 10, 35);
        pbf_press_button(context, BUTTON_A, 10, 35);
        pbf_press_button(context, BUTTON_DOWN, 10, 35);
        pbf_press_button(context, BUTTON_DOWN, 10, 35);
        pbf_press_button(context, BUTTON_DOWN, 10, 35);
        pbf_press_button(context, BUTTON_DOWN, 10, 35);
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        pbf_press_button(context, BUTTON_RIGHT, 10, 35);
        pbf_press_button(context, BUTTON_A, 10, 35);
        pbf_press_button(context, BUTTON_L, 10, 35);
        pbf_press_button(context, BUTTON_A, 10, 35);
        pbf_press_button(context, BUTTON_ZL, 10, 35);
        if(leftBox)pbf_press_button(context, BUTTON_R, 10, 35);


        std::swap(left, right);

        cursor = {0, 0};

        left.update_stats();
        right.update_stats();

        return true;
    }



    size_t runs = 0;
    for(; runs < 30; runs++){
        std::shared_ptr<Pokemon> highestLeft = left.grid[0][0];
        std::shared_ptr<Pokemon> lowestRight = right.grid[0][0];

        size_t left_row = 0;
        size_t left_col = 0;
        size_t right_row = 0;
        size_t right_col = 0;

        for(size_t i = 1; i < 30; i++){
            auto& leftPokemon = left.grid[(size_t)i / 6][(size_t)i % 6];
            if(highestLeft != nullptr && (leftPokemon == nullptr || !(*leftPokemon <= *highestLeft))) {
                highestLeft = leftPokemon;
                left_row = i/6;
                left_col = i%6;
            }

            auto& rightPokemon = right.grid[(size_t)i / 6][(size_t)i % 6];
            if (lowestRight==nullptr || (rightPokemon!=nullptr && *rightPokemon < *lowestRight)) {
                lowestRight = rightPokemon;
                right_row = i/6;
                right_col = i%6;
            }
        }

        if(highestLeft!=nullptr)highestLeft->log_details(&env, true);
        if(lowestRight!=nullptr)lowestRight->log_details(&env, true);

        if(lowestRight==nullptr || (highestLeft!=nullptr && *(highestLeft)<=*(lowestRight)))break;


        // set up easy swapping
        if (right.grid[right_row][right_col] == nullptr && left.grid[left_row][left_col] == nullptr)continue;
        else if(!leftBox && right.grid[right_row][right_col] == nullptr){
            pbf_press_button(context, BUTTON_L, 10, 150);
            leftBox = true;
        }else if (leftBox && left.grid[left_row][left_col] == nullptr){
            pbf_press_button(context, BUTTON_R, 10, 150);
            leftBox = false;
        }


        if(leftBox){
            move_cursor_to(env, context, cursor, {left_row, left_col});
            home_pick_up_pokemon(env, context, {left_row, left_col});
            pbf_press_button(context, BUTTON_R, 10, 35);
            move_cursor_to(env, context, {left_row,left_col}, {right_row, right_col});
            pbf_press_button(context, BUTTON_Y, 10, 40);
            cursor = {right_row,right_col};
            leftBox=false;
        }else{
            move_cursor_to(env, context, cursor, {right_row, right_col});
            home_pick_up_pokemon(env, context, {right_row, right_col});
            pbf_press_button(context, BUTTON_L, 10, 35);
            move_cursor_to(env, context, {right_row,right_col}, {left_row, left_col});
            pbf_press_button(context, BUTTON_Y, 10, 40);
            cursor = {left_row,left_col};
            leftBox=true;
        }
        std::swap(left.grid[left_row][left_col], right.grid[right_row][right_col]);
        if(lowestRight!=nullptr){
            left.grid[left_row][left_col]->current_col = right_col;
            left.grid[left_row][left_col]->current_row = right_row;
            left.grid[left_row][left_col]->current_box--;
        }else{
            pbf_press_button(context, BUTTON_B, 10, 35);
        }
        if(highestLeft!=nullptr){
            right.grid[right_row][right_col]->current_col = left_col;
            right.grid[right_row][right_col]->current_row = left_row;
            right.grid[right_row][right_col]->current_box++;
        }else{
            pbf_press_button(context, BUTTON_B, 10, 35);
        }

        left.update_stats();
        right.update_stats();


    }

    if(leftBox)pbf_press_button(context, BUTTON_R, 10, 35);

    return runs>0;
}

bool home_read_main_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string target){
    ImageFloatBox menu_box(0.055, 0.015, 0.25, 0.06); // Header box

    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    std::ostringstream ss;

    context.wait_for_all_requests();
    box_render.add(COLOR_GREEN, menu_box);
    std::string menu_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), menu_box)));
    env.console.log(menu_name);
    while(menu_name==""){
        env.console.log("menu_name==\"\"");
        pbf_wait(context,500ms);
        context.wait_for_all_requests();
        box_render.add(COLOR_GREEN, menu_box);
        menu_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), menu_box)));
        env.console.log(menu_name);
    }
    box_render.clear();
    env.console.log(menu_name);
    env.console.log(std::to_string(menu_name==target));
    return menu_name==target;
}

bool home_read_filter_submenu(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string target){
    ImageFloatBox filter_menu(0.65, 0.095, 0.295, 0.06); // Header box
    ImageFloatBox filter_menu_markings(0.65, 0.325, 0.295, 0.06); // Header box

    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    std::ostringstream ss;

    context.wait_for_all_requests();
    box_render.add(COLOR_GREEN, filter_menu);
    std::string submenu_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), filter_menu)));
    size_t count = 0;
    while(submenu_name!=target){

        // This next block checks for the only menu that does not extend to the top of the screen, the markings menu. Sometimes the options can be greyed out,
        // which results in weird behavior. This next block only allows the menu to back out if a menu has been selected successfully.
        box_render.add(COLOR_GREEN, filter_menu_markings);
        context.wait_for_all_requests();
        std::string markings = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), filter_menu_markings)));
        if(markings=="Search by markings"){
            if(target=="Search by markings"){
                return true;
            }else{
                pbf_press_button(context, BUTTON_B, 10, 50);
            }
        }

        // This block also checks for unexpected behavior, might be unnecessary with the addition of the above block
        if(count==5){
            context.wait_for_all_requests();
            submenu_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), filter_menu)));
            if(submenu_name!="Filters"){
                pbf_press_button(context, BUTTON_X,10, 50);
            }
        }


        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        context.wait_for_all_requests();
        submenu_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), filter_menu)));
        count++;
    }
    box_render.clear();
    return true;
}

void sv_run_ace(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ImageFloatBox sv_battle_button(0.82, 0.655, 0.1, 0.055);
    ImageFloatBox sv_keep_current_pokemon(0.645, 0.615, 0.2, 0.055);
    ImageFloatBox sv_move_1_pp(0.91, 0.615, 0.03, 0.0355);
    ImageFloatBox sv_b_back(0.95, 0.95, 0.04, 0.035);
    ImageFloatBox evolve_message(0.28, 0.76, 0.065, 0.055);
    ImageFloatBox evolve_message2(0.28, 0.76, 0.15, 0.055);
    ImageFloatBox evolve_message3(0.28, 0.76, 0.45, 0.115);
    ImageFloatBox white_dialog_checker(0.853, 0.825, 0.019, 0.05);
    ImageFloatBox fainted_checker(0.19, 0.22, 0.051, 0.03);

    VideoOverlaySet box_render(env.console);
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonSV::DialogBoxDetector dialog_detector;
    PokemonSV::DialogArrowDetector arrow_detector(Color(255,255,255), white_dialog_checker);

    int no_response_check = 0;
    int failsafe_checks = 0;
    bool secondary_move = false;
    while(true){
        screen = env.console.video().snapshot();
        box_render.add(COLOR_BLUE, sv_battle_button);
        std::string text1 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, sv_battle_button)));
        env.console.log(text1);
        box_render.add(COLOR_BLUE, sv_keep_current_pokemon);
        std::string text2 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, sv_keep_current_pokemon)));
        env.console.log(text2);
        box_render.add(COLOR_BLACK, sv_b_back);
        std::string text3 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, sv_b_back)));
        env.console.log(text3);
        std::string text4 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, evolve_message)));
        env.console.log(text4);
        std::string text5 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, fainted_checker)));
        env.console.log(text5);
        std::string text_box = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, evolve_message3)));
        env.console.log(text_box);
        box_render.add(COLOR_WHITE, white_dialog_checker);
        if(text5 == "FAINTED"||failsafe_checks==5){
            throw;
        }
        if(text_box == "| hope to see you in the tournament again soon"){
            return;
        }
        if(text4 == "What?"){
            pbf_press_button(context, BUTTON_A, 10, 90);
            std::string text6 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), evolve_message2)));
            env.console.log(text6);
            while(text6!="Congratulations!" && text6!="What? You gotta"){
                pbf_wait(context, 500ms);
                text6 = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), evolve_message2)));
                env.console.log(text6);
            }
            // ___ wants to learn the move ___
            pbf_press_button(context, BUTTON_A, 10, 90);
            std::string evo_text;
            do{
                context.wait_for_all_requests();
                box_render.add(COLOR_GREEN, evolve_message3);
                screen = env.console.video().snapshot();
                evo_text = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(screen, evolve_message3)));
                env.console.log(evo_text.c_str() + evo_text.find(' ') + 1);
                if(std::strncmp(evo_text.c_str() + evo_text.find(' ') + 1, "wants to learn the move", 23) == 0 || std::strncmp(evo_text.c_str() + evo_text.find(' ') + 1, "learned", 7) == 0){
                    pbf_press_button(context, BUTTON_B, 10, 90);
                }
                pbf_wait(context, 1000ms);
            }while((std::strncmp(evo_text.c_str() + evo_text.find(' ') + 1, "wants to learn the move", 23) == 0 || std::strncmp(evo_text.c_str() + evo_text.find(' ') + 1, "learned", 7)== 0));

        }else if(text1 == "Battle"){
            pbf_press_button(context, BUTTON_A, 10, 90);
            box_render.add(COLOR_RED, sv_move_1_pp);
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
            FloatPixel image_value = image_stats(extract_box_reference(screen, sv_move_1_pp)).average;
            env.console.log(std::to_string(image_value.r));
            if(image_value.r>100&&!secondary_move){
                secondary_move=true;
                pbf_press_button(context, BUTTON_DOWN, 10, 50);
            }
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_wait(context, 22000ms);
            failsafe_checks=0;
        }else if(text2 == "Keep current "+STRING_POKEMON){
            env.console.log("Keep current");
            pbf_press_button(context, BUTTON_B, 10, 50);
            pbf_wait(context, 8000ms);
            failsafe_checks=0;
        }else if (text3 == "Back"){
            pbf_mash_button(context, BUTTON_B, 600);
        }else if(dialog_detector.detect(screen)){
            pbf_press_button(context, BUTTON_A, 10, 250);
            failsafe_checks=0;
        }else if(arrow_detector.detect(env.console.video().snapshot())){
            pbf_press_button(context, BUTTON_A, 10, 250);
            failsafe_checks=0;
        }else if (no_response_check>25){
            no_response_check = 0;
            failsafe_checks++;
            pbf_press_button(context, BUTTON_B, 10, 250);
        }else{
            no_response_check++;
        }
        context.wait_for_all_requests();
        pbf_wait(context, 600ms);
    }
}

void sv_run_enrichment(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string sv_box_name, int max_runs){
    ImageFloatBox sv_box_name_read(0.35, 0.115, 0.2, 0.05);

    VideoOverlaySet box_render(env.console);
    VideoSnapshot screen = env.console.video().snapshot();
    pbf_wait(context, 5000ms);
    pbf_press_button(context, BUTTON_A, 10, 150*25); //open the menu out of the loop


    pbf_press_button(context, BUTTON_X, 10, 100); //open the menu out of the loop

    for(int runs = 0; runs < max_runs; runs++){
        pbf_press_button(context, BUTTON_RIGHT, 10, 150);
        pbf_press_button(context, BUTTON_DOWN, 10, 80);
        pbf_press_button(context, BUTTON_A, 10, 150);

        box_render.add(COLOR_GREEN, sv_box_name_read);
        std::string box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), sv_box_name_read)));
        env.console.log(box_name);
        while(box_name!=sv_box_name){   // navigate to correct box
            pbf_press_button(context, BUTTON_L, 10, 80);
            context.wait_for_all_requests();
            box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), sv_box_name_read)));
            env.console.log(box_name);
        }

        for(int i = 0; i < runs; i++){  // navigate to correct column
            pbf_press_button(context, BUTTON_RIGHT, 10, 50);
        }

        pbf_press_button(context, BUTTON_MINUS, 10, 50);    // multi-select
        for(int i =0; i < 5; i++){
            pbf_press_button(context, BUTTON_DOWN, 10, 30);
        }
        pbf_press_button(context, BUTTON_A, 10, 50);

        pbf_press_button(context, BUTTON_DOWN, 10, 50); // place in party
        for(int i = 0; i < runs+1; i++){
            pbf_press_button(context, BUTTON_LEFT, 10, 50);
        }
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_B, 10, 300);
        pbf_press_button(context, BUTTON_B, 10, 300);
        pbf_mash_button(context, BUTTON_A, 25000ms);
        box_render.clear();
        context.wait_for_all_requests();

        try{
            sv_run_ace(env, context);

            // put pokemon away, back out to main menu and reposition cursor over first pokemon
            pbf_press_button(context, BUTTON_A, 10, 300);
            pbf_press_button(context, BUTTON_X, 10, 150);
            pbf_press_button(context, BUTTON_A, 10, 300);
            context.wait_for_all_requests();

            box_render.add(COLOR_GREEN, sv_box_name_read);
            box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), sv_box_name_read)));
            env.console.log(box_name);
            while(box_name!=sv_box_name){   // navigate to correct box
                pbf_press_button(context, BUTTON_L, 10, 80);
                context.wait_for_all_requests();
                box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), sv_box_name_read)));
                env.console.log(box_name);
            }
            pbf_press_button(context, BUTTON_LEFT, 10, 50);    // move to party
            pbf_press_button(context, BUTTON_DOWN, 10, 50);    // multi-select
            pbf_press_button(context, BUTTON_MINUS, 10, 50);    // multi-select
            for(int i =0; i < 5; i++){
                pbf_press_button(context, BUTTON_DOWN, 10, 30);
            }
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_UP, 10, 50);
            pbf_press_button(context, BUTTON_RIGHT, 10, 50);
            for(int i = 0; i < runs; i++){  // navigate to correct column
                pbf_press_button(context, BUTTON_RIGHT, 10, 50);
            }
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_B, 10, 300);
            pbf_press_button(context, BUTTON_LEFT, 10, 150);

            //save
            pbf_press_button(context, BUTTON_R, 10, 300);
            pbf_press_button(context, BUTTON_A, 10, 150);
            pbf_wait(context, 6000ms);
            box_render.clear();
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_B, 10, 300);
        }
        catch(...){
            // env.console.log(e.what());
            // go home, reboot, and retry
            pbf_press_button(context, BUTTON_HOME,10, 50);
            pbf_press_button(context, BUTTON_X, 10, 20);
            pbf_press_button(context, BUTTON_A, 10, 320);
            pbf_press_button(context, BUTTON_A, 10, 3150);
            pbf_press_button(context, BUTTON_A, 10, 3150);

            runs--;
        }



    }
}

// This function is run in PLA. It is used to scroll left or right boxes to get to a target box
void pla_navigate_to_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t target){

}

void switch_close_game_and_open(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string game){
    ImageFloatBox switch_game_checker(0, 0.2, 1, 0.06);
    VideoOverlaySet box_render(env.console);

    pbf_press_button(context, BUTTON_HOME, 10, 80);
    pbf_press_button(context, BUTTON_X, 10, 80);
    pbf_press_button(context, BUTTON_A, 10, 240);

    for(int i = 0; i < 12; i++){
        box_render.add(COLOR_GREEN, switch_game_checker);
        context.wait_for_all_requests();
        std::string box_name = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), switch_game_checker)));
        if (box_name[0] != 'P'){
            size_t first_space = box_name.find(' ');
            if (first_space != std::string::npos && first_space + 1 < box_name.size()){
                box_name = box_name.substr(first_space + 1);
            }
        }

        env.console.log(box_name);

        if(box_name==game){
            pbf_press_button(context, BUTTON_A, 10, 150*15);
            pbf_press_button(context, BUTTON_A, 10, 150*15);
            box_render.clear();
            return;
        }else{
            pbf_press_button(context, BUTTON_RIGHT, 10, 30);

        }
    }
    box_render.clear();
    throw;
}

void Enrichment::block1(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::vector<Game>& game_list){
    ImageFloatBox game_checker(0.0455, 0.244, 0.442, 0.057);
    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    std::ostringstream ss;

    // Set up Pokémon Home
    bool setup = true;
    if(!SKIP_SETUP){

        HomeApplicationWatcher homeWatcher(COLOR_BLUE);
        int ret = wait_until(
            env.console, context, 5000ms,
            {
                homeWatcher
            }
        );

        switch(ret){
        case 0:
            env.console.log("Found Pokemon Home appliciation open");
            send_program_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                COLOR_GREEN,
                "Found Pokemon Home application",
                {}, "",
                screen
            );
            pbf_press_button(context, BUTTON_B, 10, 35);
            break;
        default:
            env.console.log("Did not find Pokemon Home application open");
            send_program_notification(
                env, NOTIFICATION_ERROR_FATAL,
                COLOR_RED,
                "Did not find Pokemon Home application",
                {}, "",
                screen
            );
            switch_close_game_and_open(env, context, "Pokémon HOME");
            break;
        }


        if(DISPOSE_GOS)home_dispose_of_go(env, context);

        if(EMERGENCY_DELOAD){
            home_navigate_to_game(env, context, game_list[0]);
            home_put_away_pokemon(env,context, game_list[0],!NORMAL_DELOAD);
        }
        do{
            // setup=initialize_home(env, context);
        }while(!setup);

    }
}

void Enrichment::block2(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool& started, bool& swaps_made){
    Game pokemon_home("Start without connecting a game", -1, false);
    home_navigate_to_game(env, context, pokemon_home);
    // bool skips_made = false;

    BoxLayout Home;

    size_t start_box = HOME_FIRST_BOX;
    size_t end_box = HOME_LAST_BOX;

    do {
        swaps_made = false;

        while (start_box < end_box && Home.is_sorted(start_box)) {
            ++start_box;
        }
        if(start_box!=HOME_FIRST_BOX){
            start_box--;
        }

        if (start_box >= end_box){
            if(started) break; // All boxes are sorted
            else{
                start_box = HOME_FIRST_BOX;
            }
        }

        env.console.log("Starting from box " + std::to_string(start_box));

        home_navigate_to_box(env, context, start_box, true);
        std::pair<size_t,size_t> cursor = {0,0};
        PokemonBox* left = new PokemonBox(home_build_box(env, context, cursor, start_box));
        PokemonBox* right = nullptr;

        Home.add_box(start_box, left);

        pbf_press_button(context, BUTTON_R, 10, 150); // Move to the right box

        for (size_t left_box = start_box; left_box < end_box; ++left_box) {
            size_t right_box = left_box + 1;

            if(left_box%10==0){
                send_program_notification(
                    env, NOTIFICATION_ERROR_FATAL,
                    COLOR_GREEN,
                    "scanned up to box "+std::to_string(left_box),
                    {}, "",
                    {}
                    );
            }

            if (right_box > HOME_LAST_BOX) break; // Prevent out-of-bounds access


            env.console.log("Checking boxes " + std::to_string(left_box) + " and " + std::to_string(right_box));

            env.console.log("box_sorted[" + std::to_string(left_box) + "] = " + (Home.is_sorted(left_box) ? "true" : "false"));
            env.console.log("box_sorted[" + std::to_string(right_box) + "] = " + (Home.is_sorted(right_box) ? "true" : "false"));

            // Load the boxes
            if(Home.get_box(right_box)==nullptr){
                right = new PokemonBox(home_build_box(env, context, cursor, right_box));
                Home.add_box(right_box, right);
            }else{
                right = Home.get_box(right_box);
            }

            env.console.log(std::to_string(Home.is_sorted(right_box)));


            // Run easy swaps, if possible
            bool swapped = home_make_easy_swaps(env, context, *left, *right, cursor, false);

            if(!swapped && !Home.is_sorted(left_box)){
                pbf_press_button(context, BUTTON_L, 10, 35);
                home_sort_box(env, context, *left, cursor);
                pbf_press_button(context, BUTTON_R, 10, 150);
            }else if(swapped){
                Home.set_sorted(right_box, false);
            }
            Home.set_sorted(left_box, !swapped);

            // Before moving on, make sure the blanks are where they should be. If the next function returns false, we have a huge problem.
            env.console.log("Validating blanks");
            move_cursor_to(env, context, cursor, {0,0});
            cursor = {0,0};
            if(!home_reconcile_blanks(env, context, *Home.get_box(right_box))){
                env.console.log("Validation failed. Rebuilding box.");
                PokemonBox* temp = new PokemonBox(home_build_box(env, context, cursor, left_box+1));
                Home.update_box(left_box+1, temp); // Yes this is correct. left_box has not been incremented yet
            }else{
                env.console.log("Validation succeeded. Continuing.");
            }
            right = left;

            pbf_press_button(context, BUTTON_R, 10, 150);

        }


        send_program_notification(
            env, NOTIFICATION_ERROR_FATAL,
            COLOR_GREEN,
            "Completed Left Scan",
            {}, "",
            {}
            );

        started=true;

        while (end_box < start_box && Home.is_sorted(end_box)) {
            --end_box;
        }
        if(end_box!=HOME_LAST_BOX)end_box++;

        if (end_box <= start_box){
            if(started) break; // All boxes are sorted
            else{
                end_box = HOME_LAST_BOX;
            }
        }

        home_navigate_to_box(env, context, end_box, true);

        pbf_press_button(context, BUTTON_L, 10, 150); // Move to the right box

        for (size_t right_box = end_box; right_box > start_box; --right_box) {
            size_t left_box = right_box - 1;

            if (left_box < start_box) break; // Prevent out-of-bounds access

            if(right_box%10==0){
                send_program_notification(
                    env, NOTIFICATION_ERROR_FATAL,
                    COLOR_GREEN,
                    "scanned down to box "+std::to_string(right_box),
                    {}, "",
                    {}
                    );
            }

            env.console.log("Checking boxes " + std::to_string(left_box) + " and " + std::to_string(right_box));

            env.console.log("box_sorted[" + std::to_string(left_box) + "] = " + (Home.is_sorted(left_box) ? "true" : "false"));
            env.console.log("box_sorted[" + std::to_string(right_box) + "] = " + (Home.is_sorted(right_box) ? "true" : "false"));

            // Load the boxes
            if(Home.get_box(left_box)==nullptr){
                left = new PokemonBox(home_build_box(env, context, cursor, left_box));
                Home.add_box(left_box, left);
            }else{
                left = Home.get_box(left_box);
            }

            bool swapped = home_make_easy_swaps(env, context, *left, *right, cursor, true);
            pbf_press_button(context, BUTTON_L, 10, 35);

            if(!swapped && !Home.is_sorted(right_box)){
                home_sort_box(env, context, *right, cursor);
            }else if(swapped){
                Home.set_sorted(left_box, false);
            }
            pbf_press_button(context, BUTTON_L, 10, 150);

            Home.set_sorted(right_box, !swapped);

            env.console.log("Validating blanks");
            move_cursor_to(env, context, cursor, {0,0});
            cursor = {0,0};
            if(!home_reconcile_blanks(env, context, *Home.get_box(left_box))){
                env.console.log("Validation failed. Rebuilding box.");
                PokemonBox* temp = new PokemonBox(home_build_box(env, context, cursor, right_box-1)); // Yes this is correct. left_box has not been incremented yet
                Home.update_box(right_box-1, temp);
            }else{
                env.console.log("Validation succeeded. Continuing.");
            }
            pbf_press_button(context, BUTTON_L, 10, 150);
        }

        left = right;

        move_cursor_to(env, context, home_locate_home_position(env, context), {0,0});

        send_program_notification(
            env, NOTIFICATION_ERROR_FATAL,
            COLOR_GREEN,
            "Completed Right Scan",
            {}, "",
            {}
            );
    } while (!Home.important_sorted(HOME_FIRST_BOX, HOME_LAST_BOX));
    home_exit_home(env, context);
}

void Enrichment::block3(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::vector<Game>& game_list){
    for(auto game: game_list){
        int pokemon = 30;
        if(!SKIP_SETUP)home_navigate_to_game(env, context, game);
        do{
            switch(game.index){
                case 0:
                    if(!SKIP_SETUP)pokemon = home_fill_boxes_to_game(env, context, game, SV_BOX_NAME);
                    if(pokemon==0)continue;
                    switch_close_game_and_open(env, context, game.name);
                    sv_run_enrichment(env,context, SV_BOX_NAME, std::ceil(pokemon/6));
                    send_program_notification(
                        env, NOTIFICATION_ERROR_FATAL,
                        COLOR_GREEN,
                        "Ran SV Enrichment on up to 30 Pokemon",
                        {}, "",
                        {}
                        );
                default:
                    switch_close_game_and_open(env, context, game.name);
            }
            switch_close_game_and_open(env, context, "Pokémon HOME");
            home_navigate_to_game(env, context, game);
            home_put_away_pokemon(env, context, game, EMERGENCY_DELOAD);
        }while(pokemon>=30);
    }
}

// This function is run in Pokémon Home, and should be used when wanting to transfer all Pokémon from a game back to home.
// Requires env, context
void Enrichment::home_put_away_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Game& game, bool emergency = false){

    home_navigate_to_box(env,context, HOME_FIRST_BOX);
    // move to the non-Home side of the boxes
    for(int i = 0; i < 6; i++){
        pbf_press_button(context, BUTTON_RIGHT, 10, 30);
    }
    pbf_press_button(context, BUTTON_DOWN, 10, 30);
    switch(game.index){
        case 0:
            home_navigate_to_box_secondary(env, context, SV_BOX_NAME);
        default:
            home_navigate_to_box_secondary(env, context, SV_BOX_NAME);
    }
    pbf_press_button(context, BUTTON_UP, 10, 30);
    // move back to the Home side of the boxes
    pbf_press_button(context, BUTTON_LEFT, 10, 30);
    size_t current_box = HOME_FIRST_BOX;
    // For each Pokémon in the box, increment it's circle marking and find it a place to be put into
    try{
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 6; j++){
                ImageFloatBox slot_box(0.55 + (0.072 * j), 0.2 + (0.1035 * i), 0.03, 0.057);
                //checking color to know if a pokemon is on the slot or not
                if((int)image_stddev(extract_box_reference(env.console.video().snapshot(), slot_box)).sum() < 5)continue;

                std::pair<size_t,size_t> target_pos = home_locate_empty_position(env, context, &current_box, HOME_LAST_BOX);
                home_move_pokemon_from_game(env, context, target_pos, {i,j}, emergency);
            }
        }
        pbf_press_button(context, BUTTON_DOWN, 10, 30);
        pbf_press_button(context, BUTTON_DOWN, 10, 30);
        pbf_press_button(context, BUTTON_DOWN, 10, 30);
        pbf_press_button(context, BUTTON_RIGHT, 10, 30);

        home_exit_home(env, context);
    }catch(...){
        env.console.log("Error, ran out of space.");
    }

}

void Enrichment::home_dispose_of_go(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::vector<int> blacklist = {144, 145, 146, 150, 151, 243, 244, 245, 249, 250, 251, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489,490,491,492,493,494,638,639,640,641,642,643,644,645,646,647,648,649,666,676,716,717,718,772,773,785,786,787,788,789,790,791,792,793,888,889,890,891,892,893,894,895,896,897,898,905,999,1000,1001,1002,1003,1004,1007,1008,1009,1010,1014,1015,1016,1017,1021,1022,1023,1024,1025}; // Take out legendaries, etc. that need to be preserved

    std::vector<int> lv10_blacklist = {2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18, 20, 22, 24, 28, 30, 31, 33, 34, 42, 44, 45, 47, 49, 51, 53, 55, 57, 61, 62, 64, 65, 65, 67, 68, 68, 70, 71, 73, 75, 76, 76, 76, 78, 80, 82, 85, 87, 89, 93, 94, 94, 97, 99, 101, 105, 106, 107, 110, 112, 117, 119, 124, 125, 126, 130, 139, 141, 148, 149, 153, 154, 156, 157, 157, 159, 160, 162, 164, 166, 168, 171, 178, 180, 181, 182, 184, 186, 186, 188, 189, 195, 202, 205, 210, 217, 219, 221, 224, 229, 230, 230, 232, 237, 247, 248, 253, 254, 256, 257, 259, 260, 262, 264, 266, 267, 268, 269, 271, 272, 274, 275, 277, 279, 281, 282, 284, 286, 288, 289, 291, 292, 294, 295, 297, 305, 306, 308, 310, 317, 319, 321, 323, 326, 329, 330, 332, 334, 340, 342, 344, 346, 348, 354, 356, 362, 364, 365, 372, 373, 375, 376, 388, 389, 391, 392, 394, 395, 397, 398, 400, 402, 404, 405, 409, 411, 414, 415, 416, 419, 421, 423, 426, 432, 435, 437, 444, 445, 450, 452, 454, 457, 460, 462, 464, 464, 466, 466, 467, 468, 473, 475, 477, 496, 497, 499, 500, 502, 503, 503, 505, 507, 508, 510, 520, 521, 523, 525, 526, 526, 530, 533, 534, 534, 536, 537, 541, 544, 545, 552, 553, 555, 558, 560, 563, 565, 567, 569, 571, 575, 576, 578, 579, 581, 583, 584, 586, 591, 593, 596, 598, 600, 601, 603, 604, 606, 608, 609, 611, 612, 614, 620, 623, 625, 628, 630, 634, 635, 637, 651, 652, 654, 655, 657, 658, 660, 662, 663, 665, 666, 668, 670, 671, 673, 675, 678, 680, 687, 689, 691, 693, 697, 699, 705, 706, 706, 713, 715, 723, 724, 724, 726, 727, 729, 730, 732, 733, 735, 737, 738, 743, 745, 748, 750, 752, 754, 756, 758, 760, 762, 763, 768, 770, 783, 784, 790, 791, 791, 792, 792, 811, 812, 814, 815, 817, 818, 820, 822, 823, 825, 826, 828, 830, 832, 834, 836, 838, 839, 844, 847, 849, 851, 857, 858, 860, 861, 862, 862, 863, 864, 866, 879, 886, 887, 901, 907, 908, 910, 911, 913, 914, 916, 918, 920, 922, 923, 925, 927, 929};

    ImageFloatBox national_dex_number_box(0.448, 0.245, 0.049, 0.04); //pokemon national dex number pos
    ImageFloatBox nature_box(0.157, 0.783, 0.212, 0.042); // Nature box
    ImageFloatBox ot_id_box(0.782, 0.719, 0.193, 0.046); // OT ID box
    ImageFloatBox level_box(0.546, 0.099, 0.044, 0.041); // Level box
    Game temp_pla("Pokémon Legends: Arceus",1,false);
    Game pokemon_home("Start without connecting a game", -1, false);
    home_navigate_to_game(env, context, temp_pla);

    pbf_press_button(context, BUTTON_LEFT, 10, 30);

    size_t temp_box = PLA_FIRST_BOX;
    bool more_go = false;
    home_navigate_to_box_secondary(env, context, temp_box);
    try{
        do{
            std::pair<size_t,size_t> next_spot = home_locate_empty_position_secondary(env, context, &temp_box, PLA_LAST_BOX, false);

            pbf_press_button(context, BUTTON_X, 10, 100);
            pbf_press_button(context, BUTTON_X, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            home_read_filter_submenu(env, context, "Search by markings"); // find markings menu
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_UP, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            home_read_filter_submenu(env, context, "Search by original game"); // find origin markings menu
            pbf_press_button(context, BUTTON_UP, 10, 30);
            pbf_press_button(context, BUTTON_UP, 10, 30);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            home_read_filter_submenu(env, context, "Search by being Shiny"); // find origin markings menu
            pbf_press_button(context, BUTTON_UP, 10, 30);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            home_read_filter_submenu(env, context, "Search by compatible games"); // find origin markings menu
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_B, 10, 240);


            // Inspect for blacklisted
            pbf_press_button(context, BUTTON_A, 10, 80);
            pbf_press_button(context, BUTTON_DOWN, 10, 80);
            pbf_press_button(context, BUTTON_A, 10, 200);

            // Take note of first ID No, ot id, and nature for looping
            context.wait_for_all_requests();
            VideoSnapshot screen = env.console.video().snapshot();
            int first_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, ot_id_box), 0xff808080, 0xffffffff);
            int first_level = OCR::read_number_waterfill(env.console, extract_box_reference(screen, level_box), 0xff000000, 0xff7f7f7f);
            int first_nat_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);

            int id = first_id;
            int level = first_level;
            int nat_id = first_nat_id;
            do{
                more_go = false;
                env.console.log("Running search");
                bool blacklisted = false;
                for(auto id_p: blacklist){
                    blacklisted = blacklisted || id_p==nat_id;
                }
                for(auto id_p2: lv10_blacklist){
                    blacklisted = blacklisted || (id_p2==nat_id&&level<=10);
                }
                if(!blacklisted){
                    more_go = true;
                    env.console.log("Found Nonblacklisted");

                    // Move this into the boxes
                    pbf_press_button(context, BUTTON_B, 10, 270);
                    pbf_press_button(context, BUTTON_A, 10, 80);
                    pbf_press_button(context, BUTTON_A, 10, 150);

                    context.wait_for_all_requests();

                    std::pair<size_t, size_t> home_pos = home_locate_home_position(env, context);
                    home_move_pokemon_to_game(env, context, home_pos, next_spot);
                    break;
                }
                pbf_press_button(context, BUTTON_R, 10, 80);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, ot_id_box), 0xff808080, 0xffffffff);
                level = OCR::read_number_waterfill(env.console, extract_box_reference(screen, level_box), 0xff000000, 0xff7f7f7f);
                nat_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);
            }while(id!=first_id||level!=first_level||nat_id!=first_nat_id);
        }while(more_go);

        pbf_press_button(context, BUTTON_B, 10, 240);
        pbf_press_button(context, BUTTON_B, 10, 240);
        size_t count = 0;
        while(!home_read_main_menu(env, context, "POKEMON")){
            if(++count==5){
                pbf_press_button(context, BUTTON_B, 10, 240);
            }
            pbf_wait(context, 500ms);
        }


    }catch(...){
        env.console.log("Ran out of spaces");
    }

    home_exit_home(env, context);

    home_navigate_to_game(env, context, pokemon_home);

    more_go = false;
    try{
        pbf_press_button(context, BUTTON_X, 10, 100);
        pbf_press_button(context, BUTTON_X, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        home_read_filter_submenu(env, context, "Search by markings"); // find markings menu
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_UP, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        home_read_filter_submenu(env, context, "Search by original game"); // find origin markings menu
        pbf_press_button(context, BUTTON_UP, 10, 30);
        pbf_press_button(context, BUTTON_UP, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        home_read_filter_submenu(env, context, "Search by being Shiny"); // find origin markings menu
        pbf_press_button(context, BUTTON_UP, 10, 30);
        pbf_press_button(context, BUTTON_A, 10, 80);
        pbf_press_button(context, BUTTON_B, 10, 270);

        do{

            // Inspect for blacklisted
            pbf_press_button(context, BUTTON_A, 10, 90);
            pbf_press_button(context, BUTTON_DOWN, 10, 90);
            pbf_press_button(context, BUTTON_A, 10, 210);

            // Take note of first ID No, ot id, and nature for looping
            context.wait_for_all_requests();
            VideoSnapshot screen = env.console.video().snapshot();
            int first_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, ot_id_box), 0xff808080, 0xffffffff);
            int first_level = OCR::read_number_waterfill(env.console, extract_box_reference(screen, level_box), 0xff000000, 0xff7f7f7f);
            int first_nat_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);

            int id = first_id;
            int level = first_level;
            int nat_id = first_nat_id;
            do{
                more_go = false;
                env.console.log("Running search");
                bool blacklisted = false;
                for(auto id_p: blacklist){
                    blacklisted = blacklisted || id_p==nat_id;
                }
                for(auto id_p2: lv10_blacklist){
                    blacklisted = blacklisted || (id_p2==nat_id&&level<=10);
                }
                if(!blacklisted){
                    more_go = true;
                    env.console.log("Found Nonblacklisted");

                    // Release
                    pbf_press_button(context, BUTTON_B, 10, 280);
                    pbf_press_button(context, BUTTON_A, 10, 90);
                    pbf_press_button(context, BUTTON_UP, 10, 60);
                    pbf_press_button(context, BUTTON_UP, 10, 60);
                    pbf_press_button(context, BUTTON_A, 10, 160);
                    pbf_press_button(context, BUTTON_A, 10, 160);
                    pbf_press_button(context, BUTTON_UP, 10, 60);
                    pbf_press_button(context, BUTTON_A, 10, 160);
                    pbf_press_button(context, BUTTON_A, 10, 210);

                    context.wait_for_all_requests();

                    break;
                }
                pbf_press_button(context, BUTTON_R, 10, 90);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, ot_id_box), 0xff808080, 0xffffffff);
                level = OCR::read_number_waterfill(env.console, extract_box_reference(screen, level_box), 0xff000000, 0xff7f7f7f);
                nat_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);
            }while(id!=first_id||level!=first_level||nat_id!=first_nat_id);
        }while(more_go);

        pbf_press_button(context, BUTTON_B, 10, 240);
        pbf_press_button(context, BUTTON_B, 10, 240);
        size_t count = 0;
        while(!home_read_main_menu(env, context, "POKEMON")){
            if(++count==5){
                pbf_press_button(context, BUTTON_B, 10, 240);
            }
            pbf_wait(context, 500ms);
        }


    }catch(...){
        env.console.log("Ran out of spaces");
    }



    home_exit_home(env, context);

}

bool Enrichment::initialize_home(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    VideoOverlaySet box_render(env.console);

    // Navigate to Pokémon Home for initial setup
    Game pokemon_home("Start without connecting a game", -1, false);
    home_navigate_to_game(env, context, pokemon_home);

    home_navigate_to_box(env, context, HOME_FIRST_BOX,true);
    size_t home_box = HOME_FIRST_BOX;

    box_render.clear();

    if(WIPE_MARKINGS){
        while(home_box<=HOME_LAST_BOX){
            home_scan_box(env, context, env.console.video().snapshot(), WIPE_MARKINGS);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_R, 10, 47);
            home_box++;
        }
    }

    return home_exit_home(env, context);
}

void Enrichment::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.console.log("Opened");

    std::vector<Game> game_list = {Game("Pokémon Violet",0,false)/*,Game("Pokémon Sword",3,false),Game("Pokémon Legends: Arceus",1,false),Game("Pokémon: Let's Go, Eevee!",4,false)*/};

    // bool started = false;
    // bool swaps_made = true;

    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    std::ostringstream ss;

    PokemonHome_HomeEnvironment(env, context);

    ImageFloatBox top_white(0.36, 0.076, 0.001, 0.001);
    FloatPixel current_box_value = image_stats(extract_box_reference(screen, top_white)).average;
    // std::string help_box = sanitize_OCR(OCR::ocr_read(Language::English, extract_box_reference(env.console.video().snapshot(), top_green)));
    box_render.add(COLOR_GREEN, top_white);


    env.console.log(std::to_string(current_box_value.r)+" "+std::to_string(current_box_value.g)+" "+std::to_string(current_box_value.b));

    pbf_wait(context, 2000ms);

    context.wait_for_all_requests();
    box_render.clear();




    HomeTitleScreenWatcher titleWatcher(COLOR_BLUE);
    HomeMainMenuWatcher mainMenuWatcher(COLOR_BLUE);
    HomeGameSelectWatcher gameSelectWatcher(COLOR_BLUE);
    HomeListViewWatcher listWatcher(COLOR_BLUE);
    HomeSummaryViewWatcher summaryWatcher(COLOR_BLUE);
    HomeMarkingsViewWatcher markingsWatcher(COLOR_BLUE);
    HomeBoxViewWatcher boxWatcher(COLOR_BLUE);
    int ret = wait_until(
        env.console, context, 5000ms,
        {
            titleWatcher,
            mainMenuWatcher,
            gameSelectWatcher,
            listWatcher,
            summaryWatcher,
            markingsWatcher,
            boxWatcher
        }
    );

    switch(ret){
    case 0:
        env.console.log("Found title screen");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found title screen",
            {}, "",
            screen
        );
        break;
    case 1:
        env.console.log("Found main menu");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found main menu",
            {}, "",
            screen
            );
        break;
    case 2:
        env.console.log("Found game select screen");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found game select screen",
            {}, "",
            screen
            );
        break;
    case 3:
        env.console.log("Found list view screen");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found list view screen",
            {}, "",
            screen
            );
        break;
    case 4:
        env.console.log("Found summary view screen");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found list view screen",
            {}, "",
            screen
            );
        break;
    case 5:
        env.console.log("Found markings screen");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found markings screen",
            {}, "",
            screen
            );
        break;
    case 6:
        env.console.log("Found box view");
        send_program_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            COLOR_GREEN,
            "Found box view",
            {}, "",
            screen
            );
        break;
    default:
        env.console.log("Did not find Pokemon Home application open");
        send_program_notification(
            env, NOTIFICATION_ERROR_FATAL,
            COLOR_RED,
            "Did not find Pokemon Home application",
            {}, "",
            screen
        );
        break;
    }


    // block1(env, context, game_list);
    // block2(env, context, started, swaps_made);
    // send_program_notification(
    //         env, NOTIFICATION_ERROR_FATAL,
    //         COLOR_GREEN,
    //         "Finished Sorting Pokemon",
    //         {}, "",
    //         screen
    //     );
    // block3(env, context, game_list);
    // send_program_notification(
    //     env, NOTIFICATION_ERROR_FATAL,
    //     COLOR_GREEN,
    //     "Finished Running Enhancements",
    //     {}, "",
    //     screen
    //     );

    // block2(env, context, started, swaps_made);

    // pbf_press_button(context, BUTTON_HOME,10, 150);
    // pbf_press_button(context, BUTTON_X, 10, 20);
    // pbf_press_button(context, BUTTON_A, 10, 320);
    // pbf_press_button(context, BUTTON_A, 10, 3150);
    // pbf_press_button(context, BUTTON_A, 10, 3150);

    // send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);



}

}
}
}
