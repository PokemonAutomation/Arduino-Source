/*  Nintendo Switch Controller Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch_ControllerSettings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



struct OfficialJoyconColors{
    std::string name;
    uint32_t left_body;
    uint32_t left_buttons;
    uint32_t right_body;
    uint32_t right_buttons;
};

const std::vector<OfficialJoyconColors>& OFFICIAL_JOYCON_COLORS(){
    //  From: https://switchbrew.org/wiki/Joy-Con#Colors
    const static std::vector<OfficialJoyconColors> database{
        {"Developer Black",                     0x313131, 0x0F0F0F, 0x313131, 0x0F0F0F},

        {"Stock: Grey / Grey",                  0x828282, 0x0F0F0F, 0x828282, 0x0F0F0F},
        {"Stock: Neon Blue / Neon Red",         0x0AB9E6, 0x001E1E, 0xFF3C28, 0x1E0A0A},
        {"Stock: OLED White",                   0xE6E6E6, 0x323232, 0xE6E6E6, 0x323232},

        {"Neon Red / Neon Blue",                0xFF3C28, 0x1E0A0A, 0x0AB9E6, 0x001E1E},
        {"Blue / Neon Yellow",                  0x4655F5, 0x00000A, 0xE6FF00, 0x142800},
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


StringSelectDatabase make_JOYCON_DATABASE(){
    StringSelectDatabase database;
    database.add_entry(StringSelectEntry("Custom", "Custom"));
    for (const OfficialJoyconColors& item : OFFICIAL_JOYCON_COLORS()){
        database.add_entry(
            StringSelectEntry(item.name, item.name)
        );
    }
    return database;
}
const StringSelectDatabase& JOYCON_DATABASE(){
    static const StringSelectDatabase database = make_JOYCON_DATABASE();
    return database;
}





const EnumDropdownDatabase<ControllerType>& ControllerSettingsType_Database(){
    static const EnumDropdownDatabase<ControllerType> database({
        {ControllerType::NintendoSwitch_WirelessProController,  "pro-controller",   "Pro Controller"},
        {ControllerType::NintendoSwitch_LeftJoycon,             "left-joycon",      "Left Joycon"},
        {ControllerType::NintendoSwitch_RightJoycon,            "right-joycon",     "Right Joycon"},
    });
    return database;
}



ControllerSettingsRow::~ControllerSettingsRow(){
    official_color.remove_listener(*this);
#if 1
    right_grip.remove_listener(*this);
    left_grip.remove_listener(*this);
//    body_color.remove_listener(*this);
    button_color.remove_listener(*this);
#endif
    controller.remove_listener(*this);
}
ControllerSettingsRow::ControllerSettingsRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , name(false, LockMode::UNLOCK_WHILE_RUNNING, "", "COM3")
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
        JOYCON_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , m_pending_official_load(0)
{
    add_option(name, "Name");
    add_option(controller, "Controller");
    add_option(button_color, "Button");
    add_option(body_color, "Body");
    add_option(left_grip, "Left Grip");
    add_option(right_grip, "Right Grip");
    add_option(official_color, "Official Color");

    controller.add_listener(*this);
#if 1
    button_color.add_listener(*this);
//    body_color.add_listener(*this);
    left_grip.add_listener(*this);
    right_grip.add_listener(*this);
#endif
    official_color.add_listener(*this);
}
std::unique_ptr<EditableTableRow> ControllerSettingsRow::clone() const{
    std::unique_ptr<ControllerSettingsRow> ret(new ControllerSettingsRow(parent()));
    ret->name.set((std::string)name);
    ret->controller.set(controller);
    ret->button_color.set(button_color);
    ret->body_color.set(body_color);
    ret->left_grip.set(left_grip);
    ret->right_grip.set(right_grip);
    ret->official_color.set_by_index(official_color.index());
    return ret;
}
void ControllerSettingsRow::value_changed(void* object){
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
        ConfigOptionState state = controller == ControllerType::NintendoSwitch_WirelessProController
            ? ConfigOptionState::ENABLED
            : ConfigOptionState::HIDDEN;
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

        const OfficialJoyconColors& entry = OFFICIAL_JOYCON_COLORS()[index - 1];
        switch (controller){
        case ControllerType::NintendoSwitch_WirelessProController:{
            //  Set the grips to the joycon colors.
            left_grip.set(entry.left_body);
            right_grip.set(entry.right_body);

            //  Average the two button colors.
            uint32_t red = ((entry.left_buttons >> 16) & 0xff) + ((entry.right_buttons >> 16) & 0xff);
            uint32_t green = ((entry.left_buttons >> 8) & 0xff) + ((entry.right_buttons >> 8) & 0xff);
            uint32_t blue = ((entry.left_buttons >> 0) & 0xff) + ((entry.right_buttons >> 0) & 0xff);
            red /= 2;
            green /= 2;
            blue /= 2;
            button_color.set((red << 16) | (green << 8) | (blue << 0));

            //  Pick something for the controller body.
            body_color.set(0xd0d0d0);

            break;
        }
        case ControllerType::NintendoSwitch_LeftJoycon:
            button_color.set(entry.left_buttons);
            body_color.set(entry.left_body);
            break;
        case ControllerType::NintendoSwitch_RightJoycon:
            button_color.set(entry.right_buttons);
            body_color.set(entry.right_body);
            break;
        default:;
        }

        m_pending_official_load--;
    }catch (...){
        m_pending_official_load--;
        throw;
    }

}




ControllerSettingsTable::ControllerSettingsTable()
    : EditableTableOption_t<ControllerSettingsRow>(
        "<b>Controller Settings:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
{}
std::vector<std::string> ControllerSettingsTable::make_header() const{
    return std::vector<std::string>{
        "Name",
        "Controller",
        "Button",
        "Body",
        "Left Grip",
        "Right Grip",
        "Quick Select",
    };
}
















}
}
