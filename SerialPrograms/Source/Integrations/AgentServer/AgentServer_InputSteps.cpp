/*  Agent Server: Input Steps
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cctype>
#include <cmath>
#include <map>
#include <sstream>
#include "AgentServer_InputSteps.h"

namespace PokemonAutomation{
namespace AgentServer{

using nlohmann::json;
using namespace NintendoSwitch;



namespace{

//  Same tables as BUTTON_BITS / BUTTON_ALIASES / DPAD_POSITIONS in buttons.py.
const std::vector<std::pair<std::string, Button>>& button_names(){
    static const std::vector<std::pair<std::string, Button>> names{
        {"Y", BUTTON_Y},
        {"B", BUTTON_B},
        {"A", BUTTON_A},
        {"X", BUTTON_X},
        {"L", BUTTON_L},
        {"R", BUTTON_R},
        {"ZL", BUTTON_ZL},
        {"ZR", BUTTON_ZR},
        {"MINUS", BUTTON_MINUS},
        {"PLUS", BUTTON_PLUS},
        {"LCLICK", BUTTON_LCLICK},
        {"RCLICK", BUTTON_RCLICK},
        {"HOME", BUTTON_HOME},
        {"CAPTURE", BUTTON_CAPTURE},
    };
    return names;
}
const std::map<std::string, std::string>& button_aliases(){
    static const std::map<std::string, std::string> aliases{
        {"+", "PLUS"},
        {"START", "PLUS"},
        {"-", "MINUS"},
        {"SELECT", "MINUS"},
        {"L3", "LCLICK"},
        {"LS", "LCLICK"},
        {"LSTICK", "LCLICK"},
        {"R3", "RCLICK"},
        {"RS", "RCLICK"},
        {"RSTICK", "RCLICK"},
        {"SCREENSHOT", "CAPTURE"},
    };
    return aliases;
}

struct Direction{
    const char* name;
    int x;
    int y;
    DpadPosition dpad;
};
const std::vector<Direction>& directions(){
    static const std::vector<Direction> list{
        {"UP",          0,  1, DPAD_UP},
        {"UP_RIGHT",    1,  1, DPAD_UP_RIGHT},
        {"RIGHT",       1,  0, DPAD_RIGHT},
        {"DOWN_RIGHT",  1, -1, DPAD_DOWN_RIGHT},
        {"DOWN",        0, -1, DPAD_DOWN},
        {"DOWN_LEFT",  -1, -1, DPAD_DOWN_LEFT},
        {"LEFT",       -1,  0, DPAD_LEFT},
        {"UP_LEFT",    -1,  1, DPAD_UP_LEFT},
    };
    return list;
}
const Direction* find_direction(const std::string& name){
    for (const Direction& direction : directions()){
        if (name == direction.name){
            return &direction;
        }
    }
    return nullptr;
}


std::string trim(const std::string& text){
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos){
        return "";
    }
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}
std::string to_upper(std::string text){
    for (char& ch : text){
        ch = (char)std::toupper((unsigned char)ch);
    }
    return text;
}

//  "up-right" -> "UP_RIGHT", "UPRIGHT" -> "UP_RIGHT", "DPAD_UP" -> "UP", "a" -> "A"
std::string normalize_name(const std::string& raw){
    std::string name = to_upper(trim(raw));
    for (char& ch : name){
        if (ch == '-' || ch == ' '){
            ch = '_';
        }
    }
    if (name.starts_with("DPAD_")){
        name = name.substr(5);
    }
    for (const char* vertical : {"UP", "DOWN"}){
        for (const char* horizontal : {"LEFT", "RIGHT"}){
            std::string v = vertical, h = horizontal;
            if (name == v + h || name == h + v || name == h + "_" + v){
                return v + "_" + h;
            }
        }
    }
    return name;
}

//  Split a combination into names. A lone "+" or "-" is the PLUS/MINUS button.
void split_buttons(const json& value, std::vector<std::string>& out){
    if (value.is_null()){
        return;
    }
    if (value.is_array()){
        for (const json& item : value){
            split_buttons(item, out);
        }
        return;
    }
    if (!value.is_string()){
        throw InputError("Buttons must be a string like \"A\" or \"L+R\", or a list of names.");
    }
    std::string text = trim(value.get<std::string>());
    if (text == "+" || text == "-"){
        out.emplace_back(text);
        return;
    }
    for (char& ch : text){
        if (ch == ','){
            ch = '+';
        }
    }
    std::stringstream ss(text);
    std::string part;
    while (std::getline(ss, part, '+')){
        if (!trim(part).empty()){
            out.emplace_back(part);
        }
    }
}

std::string valid_names_message(){
    std::string buttons, dpad;
    for (const auto& item : button_names()){
        buttons += (buttons.empty() ? "" : ", ") + item.first;
    }
    for (const Direction& direction : directions()){
        dpad += (dpad.empty() ? "" : ", ") + std::string(direction.name);
    }
    return "Valid buttons: " + buttons + ", d-pad: " + dpad + ".";
}

std::string number_text(double x){
    std::ostringstream ss;
    ss << x;
    return ss.str();
}

//  Read a non-negative-or-not integer field. Accepts 5 and 5.0.
int64_t read_integer(const json& object, const char* key, int64_t default_value){
    auto iter = object.find(key);
    if (iter == object.end() || iter->is_null()){
        return default_value;
    }
    if (iter->is_number_integer()){
        return iter->get<int64_t>();
    }
    if (iter->is_number_float()){
        double x = iter->get<double>();
        if (std::isfinite(x) && x == std::floor(x)){
            return (int64_t)x;
        }
    }
    throw InputError(std::string(key) + " must be an integer.");
}

bool is_empty_buttons(const json& value){
    return value.is_null()
        || (value.is_string() && value.get<std::string>().empty())
        || (value.is_array() && value.empty());
}

}



ParsedButtons parse_buttons(const json& value){
    std::vector<std::string> names;
    split_buttons(value, names);

    ParsedButtons ret;
    int dx = 0, dy = 0;
    std::vector<std::string> seen_dpad;
    for (const std::string& raw : names){
        std::string name = trim(raw);
        std::string key = (name == "+" || name == "-") ? name : normalize_name(name);
        auto alias = button_aliases().find(key);
        if (alias != button_aliases().end()){
            key = alias->second;
        }

        bool found = false;
        for (const auto& item : button_names()){
            if (item.first == key){
                ret.buttons |= item.second;
                found = true;
                break;
            }
        }
        if (found){
            continue;
        }

        const Direction* direction = find_direction(key);
        if (direction != nullptr){
            if ((direction->x && dx && direction->x != dx) || (direction->y && dy && direction->y != dy)){
                std::string list;
                for (const std::string& s : seen_dpad){
                    list += "\"" + s + "\", ";
                }
                throw InputError("Contradictory d-pad directions: [" + list + "\"" + name + "\"]");
            }
            dx = direction->x ? direction->x : dx;
            dy = direction->y ? direction->y : dy;
            seen_dpad.emplace_back(name);
            continue;
        }

        throw InputError("Unknown button \"" + name + "\". " + valid_names_message());
    }

    if (dx || dy){
        for (const Direction& direction : directions()){
            if (direction.x == dx && direction.y == dy){
                ret.dpad = direction.dpad;
            }
        }
    }
    return ret;
}


JoystickPosition parse_stick(const json& value){
    if (value.is_null()){
        return {0, 0};
    }
    if (value.is_string()){
        std::string key = normalize_name(value.get<std::string>());
        if (key == "NEUTRAL" || key == "CENTER" || key == "NONE"){
            return {0, 0};
        }
        const Direction* direction = find_direction(key);
        if (direction == nullptr){
            std::string names;
            for (const Direction& d : directions()){
                std::string lower = d.name;
                for (char& ch : lower){
                    ch = (char)std::tolower((unsigned char)ch);
                }
                names += (names.empty() ? "" : ", ") + lower;
            }
            throw InputError(
                "Unknown stick direction \"" + value.get<std::string>() + "\". Use one of " +
                names + ", or an [x, y] pair."
            );
        }
        double length = std::hypot((double)direction->x, (double)direction->y);
        return {direction->x / length, direction->y / length};
    }
    if (value.is_array()){
        if (value.size() != 2 || !value[0].is_number() || !value[1].is_number()){
            throw InputError("A stick position needs exactly two numbers [x, y], got " + value.dump() + ".");
        }
        double x = value[0].get<double>();
        double y = value[1].get<double>();
        if (!(x >= -1.0 && x <= 1.0 && y >= -1.0 && y <= 1.0)){
            throw InputError(
                "Stick coordinates must be within [-1, 1], got (" + number_text(x) + ", " + number_text(y) + ")."
            );
        }
        return {x, y};
    }
    throw InputError("A stick position must be a direction name or an [x, y] pair.");
}


InputStep parse_step(const json& object){
    if (!object.is_object()){
        throw InputError("Each step must be an object, e.g. {\"buttons\": \"A\"}.");
    }
    static const char* FIELDS[] = {
        "buttons", "left_stick", "right_stick", "hold_ms", "release_ms", "repeat", "wait_ms"
    };
    std::string unknown;
    for (auto iter = object.begin(); iter != object.end(); ++iter){
        bool known = false;
        for (const char* field : FIELDS){
            known |= iter.key() == field;
        }
        if (!known){
            unknown += (unknown.empty() ? "'" : ", '") + iter.key() + "'";
        }
    }
    if (!unknown.empty()){
        throw InputError("Unknown input step field(s): [" + unknown + "]");
    }

    int64_t hold = read_integer(object, "hold_ms", 80);
    int64_t release = read_integer(object, "release_ms", 80);
    int64_t repeat = read_integer(object, "repeat", 1);
    int64_t wait = read_integer(object, "wait_ms", 0);
    if (hold < 0 || release < 0 || wait < 0){
        throw InputError("Durations must not be negative.");
    }
    if (repeat < 1){
        throw InputError("repeat must be at least 1.");
    }

    InputStep step;
    step.hold_ms = (uint64_t)hold;
    step.release_ms = (uint64_t)release;
    step.repeat = (uint64_t)repeat;
    step.wait_ms = (uint64_t)wait;

    json buttons = object.value("buttons", json());
    json left = object.value("left_stick", json());
    json right = object.value("right_stick", json());
    step.wait_only = is_empty_buttons(buttons) && left.is_null() && right.is_null();
    if (step.wait_only){
        return step;
    }
    if (hold == 0){
        throw InputError("hold_ms must be positive for a step that presses something.");
    }
    step.pressed = parse_buttons(buttons);
    if (!left.is_null()){
        step.left_stick = parse_stick(left);
    }
    if (!right.is_null()){
        step.right_stick = parse_stick(right);
    }
    return step;
}


uint64_t InputStep::duration_ms() const{
    if (wait_only){
        return wait_ms;
    }
    return repeat * (hold_ms + release_ms) + wait_ms;
}

std::string InputStep::describe() const{
    if (wait_only){
        return "wait " + std::to_string(wait_ms) + "ms";
    }
    std::string ret;
    if (pressed.has_buttons()){
        ret += button_to_string(pressed.buttons);
    }
    if (pressed.has_dpad()){
        ret += (ret.empty() ? "" : " + ") + std::string("d-pad ") + dpad_to_string(pressed.dpad);
    }
    auto stick_text = [](const char* side, const JoystickPosition& p){
        return std::string(side) + " stick (" + number_text(p.x) + ", " + number_text(p.y) + ")";
    };
    if (left_stick){
        ret += (ret.empty() ? "" : " + ") + stick_text("left", *left_stick);
    }
    if (right_stick){
        ret += (ret.empty() ? "" : " + ") + stick_text("right", *right_stick);
    }
    if (ret.empty()){
        ret = "neutral";
    }
    ret += " " + std::to_string(hold_ms) + "ms";
    if (repeat > 1){
        ret += " x" + std::to_string(repeat);
    }
    return ret;
}



}
}
