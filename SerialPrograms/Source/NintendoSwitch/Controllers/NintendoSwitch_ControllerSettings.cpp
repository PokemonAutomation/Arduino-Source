/*  Nintendo Switch Controller Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <set>
#include "Common/CRC32/pabb_CRC32.h"
#include "CommonFramework/Logging/Logger.h"
//#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_ControllerSettings.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include "Common/Cpp/PrettyPrint.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


uint32_t average_colors(uint32_t x, uint32_t y){
    //  Average the two button colors.
    uint32_t red = ((x >> 16) & 0xff) + ((y >> 16) & 0xff);
    uint32_t green = ((x >> 8) & 0xff) + ((y >> 8) & 0xff);
    uint32_t blue = ((x >> 0) & 0xff) + ((y >> 0) & 0xff);
    red /= 2;
    green /= 2;
    blue /= 2;
    return (red << 16) | (green << 8) | (blue << 0);
}



struct ControllerColors{
    std::string name;

    ControllerColors(std::string p_name)
        : name(std::move(p_name))
    {}
    virtual ~ControllerColors() = default;
    virtual void write_to_profile(ControllerProfile& profile, ControllerType controller) const = 0;
};

struct JoyconColors : public ControllerColors{
    uint32_t left_body;
    uint32_t left_buttons;
    uint32_t right_body;
    uint32_t right_buttons;

    JoyconColors(
        std::string p_name,
        uint32_t p_left_body,
        uint32_t p_left_button,
        uint32_t p_right_body,
        uint32_t p_right_buttons
    )
        : ControllerColors(std::move(p_name))
        , left_body(p_left_body)
        , left_buttons(p_left_button)
        , right_body(p_right_body)
        , right_buttons(p_right_buttons)
    {}

    virtual void write_to_profile(ControllerProfile& profile, ControllerType controller) const override{
        profile.official_name = name;
        switch (controller){
        case ControllerType::NintendoSwitch_WirelessProController:
        case ControllerType::NintendoSwitch_WiredProController:{
            //  Set the grips to the joycon colors.
            profile.left_grip = left_body;
            profile.right_grip = right_body;
            profile.body_color = average_colors(left_buttons, right_buttons);
            profile.button_color = average_colors(left_body, right_body);
            break;
        }
        case ControllerType::NintendoSwitch_WiredLeftJoycon:
        case ControllerType::NintendoSwitch_WirelessLeftJoycon:
            profile.button_color = left_buttons;
            profile.body_color = left_body;
            break;
        case ControllerType::NintendoSwitch_WiredRightJoycon:
        case ControllerType::NintendoSwitch_WirelessRightJoycon:
            profile.button_color = right_buttons;
            profile.body_color = right_body;
            break;
        default:;
        }
    }
};
struct ProconColors : public ControllerColors{
    uint32_t left_grip;
    uint32_t right_grip;
    uint32_t body;
    uint32_t buttons;

    ProconColors(
        std::string p_name,
        uint32_t p_left_grip,
        uint32_t p_right_grip,
        uint32_t p_body,
        uint32_t p_buttons
    )
        : ControllerColors(std::move(p_name))
        , left_grip(p_left_grip)
        , right_grip(p_right_grip)
        , body(p_body)
        , buttons(p_buttons)
    {}

    virtual void write_to_profile(ControllerProfile& profile, ControllerType controller) const override{
        profile.official_name = name;
        switch (controller){
        case ControllerType::NintendoSwitch_WirelessProController:
        case ControllerType::NintendoSwitch_WiredProController:{
            //  Set the grips to the joycon colors.
            profile.left_grip = left_grip;
            profile.right_grip = right_grip;
            profile.body_color = body;
            profile.button_color = buttons;
            break;
        }
        case ControllerType::NintendoSwitch_WiredLeftJoycon:
        case ControllerType::NintendoSwitch_WirelessLeftJoycon:
            profile.button_color = buttons;
            profile.body_color = left_grip;
            break;
        case ControllerType::NintendoSwitch_WiredRightJoycon:
        case ControllerType::NintendoSwitch_WirelessRightJoycon:
            profile.button_color = buttons;
            profile.body_color = right_grip;
            break;
        default:;
        }
    }
};

const std::vector<JoyconColors>& OFFICIAL_JOYCON_COLORS0(){
    //  From: https://switchbrew.org/wiki/Joy-Con#Colors
    const static std::vector<JoyconColors> database{
        {"Developer Black",                     0x313131, 0x0F0F0F, 0x313131, 0x0F0F0F},

        {"Stock: Grey / Grey",                  0x828282, 0x0F0F0F, 0x828282, 0x0F0F0F},
        {"Stock: Neon Blue / Neon Red",         0x0AB9E6, 0x001E1E, 0xFF3C28, 0x1E0A0A},
        {"Stock: OLED White",                   0xE6E6E6, 0x323232, 0xE6E6E6, 0x323232},

        {"Neon Red / Neon Blue",                0xFF3C28, 0x1E0A0A, 0x0AB9E6, 0x001E1E},
        {"Blue / Neon Yellow",                  0x4655F5, 0x00000A, 0xE6FF00, 0x142800},
        {"Neon Pink / Neon Green",              0xFF3278, 0x28001E, 0x1EDC00, 0x002800},
        {"Neon Green / Neon Pink",              0x1EDC00, 0x002800, 0xFF3278, 0x28001E},
        {"Neon Purple / Neon Orange",           0xB400E6, 0x140014, 0xFAA005, 0x0F0A00},
        {"Pastel Pink / Pastel Pink",           0xFFAFAF, 0x372D2D, 0xFFAFAF, 0x372D2D},
        {"Pastel Pink / Pastel Yellow",         0xFFAFAF, 0x372D2D, 0xF5FF82, 0x32332D},
        {"Pastel Purple / Pastel Green",        0xF0CBEB, 0x373037, 0xBCFFC8, 0x2D322D},

        {"Super Mario Odyssey Edition Red",                 0xE10F00, 0x280A0A, 0xE10F00, 0x280A0A},
        {"Let's Go! Pikachu and Eevee",                     0xC88C32, 0x281900, 0xFFDC00, 0x322800},
        {"Nintendo Labo Creators Contest Edition",          0xD7AA73, 0x1E1914, 0xD7AA73, 0x1E1914},
        {"Dragon Quest XI S Lotto Edition",                 0x1473FA, 0x00000F, 0x1473FA, 0x00000F},
//        {"Disney Tsum Tsum Festival Edition",               0xB400E6, 0x140014, 0xFF3278, 0x28001E},
        {"Animal Crossing: New Horizons Edition",           0x82FF96, 0x0A1E0A, 0x96F5F5, 0x0A1E28},
        {"Fortnite Wildcat Edition",                        0xFFCC00, 0x1A1100, 0x0084FF, 0x000F1E},
        {"Mario Red x Blue Edition",                        0xF04614, 0x1E1914, 0xF04614, 0x1E1914},
        {"Fortnite Fleet Force Edition",                    0x0084FF, 0x000F1E, 0xFFCC00, 0x1A1100},
        {"Legend of Zelda: Skyward Sword Edition",          0x2D50F0, 0x1E0F46, 0x500FC8, 0x00051E},
        {"Splatoon 3 OLED Edition",                         0x6455F5, 0x28282D, 0xC3FA05, 0x1E1E28},
        {"Pokémon: Scarlet x Violet OLED Edition",          0xF07341, 0x322D1E, 0x9650AA, 0x32322D},
        {"Legend of Zelda: Tears of the Kingdom Edition",   0xD2BE69, 0x32322D, 0xD2BE69, 0x32322D},
    };
    return database;
}
const std::vector<ProconColors>& OFFICIAL_PROCON_COLORS0(){
    //  From: https://gbatemp.net/threads/people-with-special-edition-pro-controllers-i-need-your-help-with-colors.627413/
    const static std::vector<ProconColors> database{
        {"Procon: Stock Black",                 0x323232, 0x323232, 0x323232, 0xFFFFFF},    //  This is just a guess.
        {"Procon: Super Smash Bros. Ultimate",  0xFFFFFF, 0xFFFFFF, 0x2D2D2D, 0xE6E6E6},
        {"Procon: Monster Hunter Rise",         0x474646, 0x474646, 0x2E2D2D, 0xE7E6E6},
        {"Procon: Monster Hunter Sunbreak",     0x464646, 0x464646, 0x2D2E2D, 0xE7E6E6},
        {"Procon: Zelda Tears of the Kingdom",  0x464646, 0xFFFFFF, 0x2D2D2D, 0xE6E6E6},
//        {"Procon: Xenoblade Chronicles 2",      0xFFFFFF, 0xFFFFFF, 0x323132, 0xFFFFFF},    //  The actual values. Switch automatically replaces grips with pink.
        {"Procon: Xenoblade Chronicles 2",      0xff3a66, 0xff3a66, 0x323132, 0xFFFFFF},    //  Grip color is a guess.
        {"Procon: Splatoon 2",                  0x1EDC00, 0xFF3278, 0x2D2D2D, 0xE6E6E6},    //  This is just a guess.
        {"Procon: Splatoon 3",                  0x6455F5, 0xC3FA05, 0x2D2D2D, 0xE6E6E6},
    };
    return database;
}
std::vector<const ControllerColors*> make_OFFICIAL_CONTROLLER_COLORS(){
    std::vector<const ControllerColors*> ret;
    for (const JoyconColors& item : OFFICIAL_JOYCON_COLORS0()){
        ret.emplace_back(&item);
    }
    for (const ProconColors& item : OFFICIAL_PROCON_COLORS0()){
        ret.emplace_back(&item);
    }
    return ret;
}
const std::vector<const ControllerColors*> OFFICIAL_CONTROLLER_COLORS(){
    static const std::vector<const ControllerColors*> database = make_OFFICIAL_CONTROLLER_COLORS();
    return database;
}

StringSelectDatabase make_CONTROLLER_DATABASE(){
    StringSelectDatabase database;
    database.add_entry(StringSelectEntry("Custom", "Custom"));
    for (const JoyconColors& item : OFFICIAL_JOYCON_COLORS0()){
        database.add_entry(
            StringSelectEntry(item.name, item.name)
        );
    }
    for (const ProconColors& item : OFFICIAL_PROCON_COLORS0()){
        database.add_entry(
            StringSelectEntry(item.name, item.name)
        );
    }
    return database;
}
const StringSelectDatabase& CONTROLLER_DATABASE(){
    static const StringSelectDatabase database = make_CONTROLLER_DATABASE();
    return database;
}





const EnumDropdownDatabase<ControllerType>& ControllerSettingsType_Database(){
    static const EnumDropdownDatabase<ControllerType> database({
        {ControllerType::NintendoSwitch_WirelessProController,  "pro-controller",   "NS1: Pro Controller"},
        {ControllerType::NintendoSwitch_WirelessLeftJoycon,     "left-joycon",      "NS1: Left Joycon"},
        {ControllerType::NintendoSwitch_WirelessRightJoycon,    "right-joycon",     "NS1: Right Joycon"},
    });
    return database;
}



ControllerSettingsRow::~ControllerSettingsRow(){
    randomize.remove_listener(static_cast<ButtonListener&>(*this));
    official_color.remove_listener(*this);
#if 1
    right_grip.remove_listener(*this);
    left_grip.remove_listener(*this);
//    body_color.remove_listener(*this);
    button_color.remove_listener(*this);
#endif
    controller.remove_listener(*this);
}
ControllerSettingsRow::ControllerSettingsRow(EditableTableOption& parent_table, bool random_profile)
    : EditableTableRow(parent_table)
    , name(false, LockMode::UNLOCK_WHILE_RUNNING, "", "COM3")
    , controller_mac_address(LockMode::UNLOCK_WHILE_RUNNING, 6, nullptr)
    , controller(
        ControllerSettingsType_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ControllerType::NintendoSwitch_WirelessProController
    )
    , button_color(
        LockMode::UNLOCK_WHILE_RUNNING,
        false,
        0x0A1E19, 0x0A1E19
    )
    , body_color(
        LockMode::UNLOCK_WHILE_RUNNING,
        false,
        0xd0d0d0, 0xd0d0d0
    )
    , left_grip(
        LockMode::UNLOCK_WHILE_RUNNING,
        false,
        0x82FF96, 0x82FF96
    )
    , right_grip(
        LockMode::UNLOCK_WHILE_RUNNING,
        false,
        0x96F5F5, 0x96F5F5
    )
    , official_color(
        CONTROLLER_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , randomize("Randomize")
    , m_pending_official_load(0)
{
    add_option(name, "Name");
    add_option(controller_mac_address, "Controller MAC Address");
    add_option(controller, "Controller");
    add_option(button_color, "Button");
    add_option(body_color, "Body");
    add_option(left_grip, "Left Grip");
    add_option(right_grip, "Right Grip");
    add_option(randomize, "Randomize");
    add_option(official_color, "Official Color");

    if (random_profile){
        set_profile(ControllerSettingsTable::random_profile(controller, nullptr));
    }

    controller.add_listener(*this);
#if 1
    button_color.add_listener(*this);
//    body_color.add_listener(*this);
    left_grip.add_listener(*this);
    right_grip.add_listener(*this);
#endif
    official_color.add_listener(*this);
    randomize.add_listener(static_cast<ButtonListener&>(*this));
}
std::unique_ptr<EditableTableRow> ControllerSettingsRow::clone() const{
    std::unique_ptr<ControllerSettingsRow> ret(new ControllerSettingsRow(parent()));
    ret->name.set((std::string)name);
    ret->controller_mac_address = controller_mac_address;
    ret->controller.set(controller);
    ret->button_color.set(button_color);
    ret->body_color.set(body_color);
    ret->left_grip.set(left_grip);
    ret->right_grip.set(right_grip);
    ret->official_color.set_by_index(official_color.index());
    return ret;
}
void ControllerSettingsRow::on_config_value_changed(void* object){
    if (object == &button_color ||
//        object == &body_color ||
        object == &left_grip ||
        object == &right_grip
    ){
        if (m_pending_official_load.load(std::memory_order_relaxed) == 0){
            official_color.set_by_index(0);
        }
        return;
    }

    if (object == &controller){
        ConfigOptionState state;
        switch (controller){
        case ControllerType::NintendoSwitch_WirelessProController:
        case ControllerType::NintendoSwitch_WiredProController:
            state = ConfigOptionState::ENABLED;
            break;
        default:
            state = ConfigOptionState::HIDDEN;
        }
        left_grip.set_visibility(state);
        right_grip.set_visibility(state);

        //  Force an update on the chosen theme.
        object = &official_color;
    }

    if (object != &official_color){
        return;
    }

    size_t index = official_color.index();
    if (index == 0){
        return;
    }

    try{
        m_pending_official_load++;

        ControllerProfile profile{};
        OFFICIAL_CONTROLLER_COLORS()[index - 1]->write_to_profile(profile, controller);
        this->set_profile(profile);

        m_pending_official_load--;
    }catch (...){
        m_pending_official_load--;
        throw;
    }

}
void ControllerSettingsRow::on_press(){
    set_profile(ControllerSettingsTable::random_profile(controller, nullptr));
}




ControllerSettingsTable::ControllerSettingsTable()
    : EditableTableOption_t<ControllerSettingsRow>(
        "<b>OEM Controller Settings:</b><br><br>"
        "The OEM controllers (official Joycons and Pro Controller) support colors. "
        "This table will let you customize those colors."
        "<br>"
        "The \"Name\" column is an arbitrary text field to help you identify which device it is. "
        "By default it uses the name of the serial port. But this can be misleading if the port has changed. "
        "<br>"
        "Do not change the MAC address as it is used to identify which device the row belongs to. "
        "Changing it will not actually change the MAC address of the device."
        "<br><br>"
        "Changes take effect after resetting the device.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
{}
std::vector<std::string> ControllerSettingsTable::make_header() const{
    return std::vector<std::string>{
        "Name",
        "MAC Address",
        "Controller",
        "Button",
        "Body",
        "Left Grip",
        "Right Grip",
        "",
        "Quick Select",
    };
}


uint32_t invert_RGB32(uint32_t x){
    uint8_t r = (uint8_t)(x >> 16);
    uint8_t g = (uint8_t)(x >>  8);
    uint8_t b = (uint8_t)(x >>  0);
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
    return
        ((uint32_t)r << 16) |
        ((uint32_t)g <<  8) |
        ((uint32_t)b <<  0);
}
bool is_white(uint32_t x){
    uint8_t r = (uint8_t)(x >> 16);
    uint8_t g = (uint8_t)(x >>  8);
    uint8_t b = (uint8_t)(x >>  0);
    return r >= 0xf0 && g >= 0xf0 && b >= 0xf0;
}
bool is_black(uint32_t x){
    uint8_t r = (uint8_t)(x >> 16);
    uint8_t g = (uint8_t)(x >>  8);
    uint8_t b = (uint8_t)(x >>  0);
    return r < 0x10 && g < 0x10 && b < 0x10;
}


ControllerProfile ControllerSettingsTable::random_profile(
    ControllerType controller,
    const uint8_t* mac_address
){
    const std::vector<const ControllerColors*>& DATABASE = OFFICIAL_CONTROLLER_COLORS();

    //  Ban all colors are the similar to the stock colors.
    static const std::set<std::string> BLACK_LIST{
        "Developer Black",
        "Stock: Grey / Grey",
        "Procon: Stock Black",
        "Procon: Monster Hunter Rise",
        "Procon: Monster Hunter Sunbreak",
    };

    ControllerProfile profile;

    uint32_t seed = 0;
    while (true){
        if (mac_address){
            pabb_crc32_buffer(&seed, mac_address, 6 * sizeof(uint8_t));
        }else{
            uint64_t seed64 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            pabb_crc32_buffer(&seed, &seed64, sizeof(seed64));
        }

        size_t index = seed % DATABASE.size();
//        cout << "index = " << index << endl;

        DATABASE[index]->write_to_profile(profile, controller);
        if (BLACK_LIST.contains(profile.official_name)){
            continue;
        }

//        cout << "name: " << profile.official_name << endl;

        if (seed & 0x80000000){
            profile.official_name.clear();
            std::swap(profile.body_color, profile.button_color);
        }
        if (seed & 0x40000000){
            profile.official_name.clear();
            profile.body_color = invert_RGB32(profile.body_color);
        }
        if (seed & 0x20000000){
            profile.official_name.clear();
            profile.left_grip = invert_RGB32(profile.left_grip);
        }
        if (seed & 0x10000000){
            profile.official_name.clear();
            profile.right_grip = invert_RGB32(profile.right_grip);
        }

        //  Run blacklists.
        if (is_black(profile.left_grip)){
            continue;
        }
        if (is_black(profile.right_grip)){
            continue;
        }

        break;
    }

//    cout << tostr_hex(profile.body_color) << endl;
//    cout << tostr_hex(profile.left_grip) << endl;
//    cout << tostr_hex(profile.right_grip) << endl;

    return profile;
}
ControllerProfile ControllerSettingsTable::get_or_make_profile(
    const uint8_t mac_address[6],
    const std::string& name,
    ControllerType controller
){
//    cout << "ControllerSettingsTable::get_or_make_profile(): " << this << endl;

    ControllerProfile profile{};

    //  Only relevant to Switch controllers.
    switch (controller){
    case ControllerType::NintendoSwitch_WiredProController:
    case ControllerType::NintendoSwitch_WirelessProController:
        controller = ControllerType::NintendoSwitch_WirelessProController;
        break;
    case ControllerType::NintendoSwitch_WiredLeftJoycon:
    case ControllerType::NintendoSwitch_WirelessLeftJoycon:
        controller = ControllerType::NintendoSwitch_WirelessLeftJoycon;
        break;
    case ControllerType::NintendoSwitch_WiredRightJoycon:
    case ControllerType::NintendoSwitch_WirelessRightJoycon:
        controller = ControllerType::NintendoSwitch_WirelessRightJoycon;
        break;
//    case ControllerType::NintendoSwitch2_WiredProController:
//    case ControllerType::NintendoSwitch2_WirelessProController:
//        controller = ControllerType::NintendoSwitch2_WirelessProController;
//        break;
//    case ControllerType::NintendoSwitch2_WiredLeftJoycon:
//    case ControllerType::NintendoSwitch2_WirelessLeftJoycon:
//        controller = ControllerType::NintendoSwitch2_WirelessLeftJoycon;
//        break;
//    case ControllerType::NintendoSwitch2_WiredRightJoycon:
//    case ControllerType::NintendoSwitch2_WirelessRightJoycon:
//        controller = ControllerType::NintendoSwitch2_WirelessRightJoycon;
//        break;
        break;
    default:
        return profile;
    }

    bool found = false;
    this->run_on_all_rows([&, controller](ControllerSettingsRow& row){
        if (row.controller_mac_address != mac_address){
            return false;
        }

        global_logger_tagged().log("ControllerSettingsTable: Found matching MAC address. Loading profile...");
        row.controller.set(controller);
        found = true;
        profile = row;
        return true;
    });

    if (found){
        return profile;
    }

    global_logger_tagged().log("ControllerSettingsTable: Creating new profile...");

    profile = random_profile(controller, mac_address);

    std::unique_ptr<ControllerSettingsRow> row(new ControllerSettingsRow(*this, false));
    row->name.set(name);
    row->controller_mac_address.set(mac_address);
    row->controller.set(controller);
    row->set_profile(profile);

    this->append_row(std::move(row));
//    PERSISTENT_SETTINGS().write();

    //  This is brutal (tech-debt). The only way to force the settings to save
    //  is to load the panel. TODO: Redesign panels to allow external editing.
    //  This is also coming in from a different thread (not the main Qt thread).
    PanelDescriptorWrapper<ConsoleSettings_Descriptor, ConsoleSettingsPanel>().make_panel()->save_settings();

    return profile;
}













}
}
