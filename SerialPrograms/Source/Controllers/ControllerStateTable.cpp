/*  Controller State Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ControllerTypeStrings.h"
#include "ControllerStateTable.h"

#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProControllerTable.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_JoyconTable.h"

#include "NintendoSwitch/Options/TurboMacroTable.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



struct ControllerTypeEntry{
    ControllerCommandTable::RowFactory factory;
    std::vector<std::string> headers;
};


std::map<ControllerClass, ControllerTypeEntry> controller_map;


const std::map<ControllerClass, ControllerTypeEntry>& make_controller_map(){
    NintendoSwitch::register_procon_tables();
    NintendoSwitch::register_joycon_tables();
    return controller_map;
}

const std::map<ControllerClass, ControllerTypeEntry>& get_controller_map(){
    static const std::map<ControllerClass, ControllerTypeEntry>& map = make_controller_map();
    return map;
}



void ControllerCommandTable::register_controller_type(
    ControllerClass type,
    RowFactory factory,
    std::vector<std::string> headers
){
    auto& map = controller_map;
    if (map.emplace(
            type, ControllerTypeEntry{factory, std::move(headers)}
        ).second
    ){
        return;
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Duplicate Controller Enum: " + std::to_string((int)type)
    );
}




void ControllerCommandTable::load_json_NS_TurboMacro(const JsonValue& json){
    using namespace NintendoSwitch;

    if (m_type != ControllerClass::NintendoSwitch_ProController){
        throw FileException(
            nullptr, nullptr,
            "Incompatible controller type.",
            ""
        );
    }

    JsonObject translated;
    translated["ControllerClass"] = CONTROLLER_CLASS_STRINGS().get_string(m_type);

    //  JSON to JSON translate to the new format.
    JsonArray history;

    for (const JsonValue& item : json.to_array_throw()){
        const JsonObject& obj = item.to_object_throw();

        const std::string& action_name = obj.get_string_throw("Action");
        const auto* entry = TurboMacroAction_Database().find_slug(action_name);
        if (entry == nullptr){
            throw ParseException("Invalid Action: " + action_name);
        }

        const JsonValue* value;

        TurboMacroAction action = (TurboMacroAction)entry->enum_value;

        if (action == TurboMacroAction::NO_ACTION){
            continue;
        }

        //  Wait has different timing fields.
        if (action == TurboMacroAction::WAIT){
            value = obj.get_value("Wait");
            if (value != nullptr && value->is_integer()){
                JsonObject out;
                out["ms"] = std::to_string(value->to_integer_default() * 8) + " ms";
                history.push_back(std::move(out));
            }
            value = obj.get_value("WaitMs");
            if (value != nullptr){
                JsonObject out;
                out["ms"] = value->to_string_throw();
                history.push_back(std::move(out));
            }
            continue;
        }

        JsonObject command;

        value = obj.get_value("Hold");
        if (value != nullptr && value->is_integer()){
            command["ms"] = std::to_string(value->to_integer_default() * 8) + " ms";
        }
        value = obj.get_value("HoldMs");
        if (value != nullptr){
            command["ms"] = value->to_string_throw();
        }
        if (value == nullptr){
            throw ParseException("Missing Duration: " + obj.dump());
        }


        switch (action){
        case TurboMacroAction::B:
            command["buttons"] = "B";
            break;
        case TurboMacroAction::A:
            command["buttons"] = "A";
            break;
        case TurboMacroAction::Y:
            command["buttons"] = "Y";
            break;
        case TurboMacroAction::X:
            command["buttons"] = "X";
            break;
        case TurboMacroAction::R:
            command["buttons"] = "R";
            break;
        case TurboMacroAction::L:
            command["buttons"] = "L";
            break;
        case TurboMacroAction::ZR:
            command["buttons"] = "ZR";
            break;
        case TurboMacroAction::ZL:
            command["buttons"] = "ZL";
            break;
        case TurboMacroAction::PLUS:
            command["buttons"] = "+";
            break;
        case TurboMacroAction::MINUS:
            command["buttons"] = "-";
            break;
        case TurboMacroAction::LEFT_JOY_CLICK:
            command["buttons"] = "LJ";
            break;
        case TurboMacroAction::RIGHT_JOY_CLICK:
            command["buttons"] = "RJ";
            break;
        case TurboMacroAction::DPADLEFT:
            command["dpad"] = "left";
            break;
        case TurboMacroAction::DPADRIGHT:
            command["dpad"] = "right";
            break;
        case TurboMacroAction::DPADUP:
            command["dpad"] = "up";
            break;
        case TurboMacroAction::DPADDOWN:
            command["dpad"] = "down";
            break;
        case TurboMacroAction::LEFT_JOYSTICK:
            value = obj.get_value("MoveDirectionX");
            if (value != nullptr){
                command["lx"] = value->to_integer_throw();
            }
            value = obj.get_value("MoveDirectionY");
            if (value != nullptr){
                command["ly"] = value->to_integer_throw();
            }
            break;
        case TurboMacroAction::RIGHT_JOYSTICK:
            value = obj.get_value("MoveDirectionX");
            if (value != nullptr){
                command["rx"] = value->to_integer_throw();
            }
            value = obj.get_value("MoveDirectionY");
            if (value != nullptr){
                command["ry"] = value->to_integer_throw();
            }
            break;
        default:;
        }

        history.push_back(std::move(command));


        JsonObject cooldown;
        value = obj.get_value("Release");
        if (value != nullptr && value->is_integer()){
            cooldown["ms"] = std::to_string(value->to_integer_default() * 8) + " ms";
        }
        value = obj.get_value("ReleaseMs");
        if (value != nullptr){
            cooldown["ms"] = value->to_string_throw();
        }
        if (value != nullptr){
            history.push_back(std::move(cooldown));
        }
    }

    translated["Schedule"] = std::move(history);

//    cout << translated.dump() << endl;

    JsonValue value(std::move(translated));
    load_json(value);
}

void ControllerCommandTable::load_json(const JsonValue& json){
    clear();

    if (json.is_array()){
        load_json_NS_TurboMacro(json);
        return;
    }

    const JsonObject& obj = json.to_object_throw();

    const JsonValue* value = obj.get_value("controller_class");
    if (value == nullptr){
        value = obj.get_value("ControllerClass");
    }
    if (value == nullptr){
        throw JsonParseException("", "ControllerClass");
    }
    const std::string& controller = value->to_string_throw();
    if (CONTROLLER_CLASS_STRINGS().get_enum(controller) != m_type){
        throw FileException(
            nullptr, nullptr,
            "Incompatible controller type.",
            ""
        );
    }

    value = obj.get_value("history");
    if (value == nullptr){
        value = obj.get_value("Schedule");
    }
    if (value == nullptr){
        throw JsonParseException("", "Schedule");
    }

    EditableTableOption::load_json(*value);
}
JsonValue ControllerCommandTable::to_json() const{
    JsonObject obj;
    obj["ControllerClass"] = CONTROLLER_CLASS_STRINGS().get_string(m_type);
    JsonArray history;
    run_on_all_rows<ControllerStateRow>([&](const ControllerStateRow& row){
        history.push_back(row.to_json());
        return false;
    });
    obj["Schedule"] = std::move(history);
    return obj;
}

std::vector<std::string> ControllerCommandTable::make_header() const{
    auto& map = get_controller_map();
    auto iter = map.find(m_type);
    if (iter != map.end()){
        return iter->second.headers;
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Unsupported Controller Type: " + CONTROLLER_CLASS_STRINGS().get_string(m_type)
    );
}
std::unique_ptr<EditableTableRow> ControllerCommandTable::make_row(){
    auto& map = get_controller_map();
    auto iter = map.find(m_type);
    if (iter != map.end()){
        return iter->second.factory(*this);
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Unsupported Controller Type: " + CONTROLLER_CLASS_STRINGS().get_string(m_type)
    );
}







ControllerCommandTables::~ControllerCommandTables(){
    m_type.remove_listener(*this);
}
ControllerCommandTables::ControllerCommandTables(
    std::string label,
    const std::vector<ControllerClass>& controller_list
)
    : GroupOption(
        std::move(label),
        LockMode::UNLOCK_WHILE_RUNNING,
        EnableMode::ALWAYS_ENABLED
    )
    , m_type(
        "<b>Controller Type:</b>",
        make_database(controller_list),
        LockMode::LOCK_WHILE_RUNNING,
        ControllerClass::NintendoSwitch_ProController
    )
    , m_table(
        ControllerClass::NintendoSwitch_ProController,
        "",
        LockMode::UNLOCK_WHILE_RUNNING
    )
{
    PA_ADD_OPTION(m_type);
    PA_ADD_OPTION(m_table);
    m_type.add_listener(*this);
}


void ControllerCommandTables::on_config_value_changed(void* object){
    m_table.clear();
    m_table.m_type = m_type;
}


EnumDropdownDatabase<ControllerClass> ControllerCommandTables::make_database(
    const std::vector<ControllerClass>& controller_list
){
    auto& map = get_controller_map();
    EnumDropdownDatabase<ControllerClass> ret;
    for (ControllerClass type : controller_list){
        if (map.find(type) == map.end()){
            throw InternalProgramError(
                nullptr,
                PA_CURRENT_FUNCTION,
                "Unsupported Controller Type: " + CONTROLLER_CLASS_STRINGS().get_string(type)
            );
        }
        const std::string& str = CONTROLLER_CLASS_STRINGS().get_string(type);
        ret.add(type, str, str, true);
    }
    return ret;
}



std::string get_joystick_direction(const JoystickPosition& position){
    std::string direction = "";
    if (position.x > 0){
        if (position.y < 0){
            // right-down
            direction = "\u2198";
        }else if (position.y == 0){
            // right
            direction = "\u2192";
        }else{ // y < STICK_CENTER
            // right-up
            direction = "\u2197";
        }
    }else if (position.x == 0){
        if (position.y < 0){
            // down
            direction = "\u2193";
        }else if (position.y == 0){
            // neutral
            direction = "neutral";
        }else{ // y < STICK_CENTER
            // up
            direction = "\u2191";
        }

    }else { // x < STICK_CENTER
        if (position.y < 0){
            // left-down
            direction = "\u2199";
        }else if (position.y == 0){
            // left
            direction = "\u2190";
        }else{ // y < STICK_CENTER
            // left-up
            direction = "\u2196";
        }
    }

    return direction;
}



}
