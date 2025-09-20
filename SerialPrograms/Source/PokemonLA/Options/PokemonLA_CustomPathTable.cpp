/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

//#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
//#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/Options/ConfigWidget.h"
//#include "CommonFramework/Globals.h"
#include "PokemonLA_CustomPathTable.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace std::chrono_literals;




const EnumDropdownDatabase<PathAction>& PathAction_Database(){
    static const EnumDropdownDatabase<PathAction> database({
        {PathAction::NO_ACTION,             "no-action",            "NO Action"},
        {PathAction::CHANGE_MOUNT,          "change-mount",         "Change Mount"},
        {PathAction::MOVE_FORWARD,          "move-forward",         "Move Forward"},
        {PathAction::MOVE_IN_DIRECTION,     "move-in-direction",    "Move in Direction"},
        {PathAction::CENTER_CAMERA,         "center-camera",        "Center Camera"},
        {PathAction::JUMP,                  "jump",                 "Jump"},
        {PathAction::WAIT,                  "wait",                 "Wait"},
        {PathAction::START_LISTEN,          "start-listen",         "Start Listen"},
        {PathAction::END_LISTEN,            "end-listen",           "End Listen"}
    });
    return database;
}
const EnumDropdownDatabase<PathMount>& PathMount_Database(){
    static const EnumDropdownDatabase<PathMount> database({
        {PathMount::NO_MOUNT,       "none",         "No Mount"},
        {PathMount::WYRDEER,        "wrydeer",      "Wrydeer"},
        {PathMount::URSALUNA,       "ursaluna",     "Ursaluna"},
        {PathMount::BASCULEGION,    "basculegion",  "Basculegion"},
        {PathMount::SNEASLER,       "sneasler",     "Sneasler"},
        {PathMount::BRAVIARY,       "braviary",     "Braviary"},
    });
    return database;
}
const EnumDropdownDatabase<PathSpeed>& PathSpeed_Database(){
    static const EnumDropdownDatabase<PathSpeed> database({
        {PathSpeed::NORMAL_SPEED,   "normal",   "Normal Speed"},
        {PathSpeed::SLOW_SPEED,     "slow",     "Slow Speed"},
        {PathSpeed::RUN,            "run",      "Run on Foot"},
        {PathSpeed::DASH,           "dash",     "Dash on Ride"},
        {PathSpeed::DASH_B_SPAM,    "mash-b",   "Dash on Braviary B Spam"},
        {PathSpeed::DIVE,           "dive",     "Dive on Braviary"},
    });
    return database;
}





CustomPathCell::~CustomPathCell(){
    m_action.remove_listener(*this);
}
void CustomPathCell::operator=(const CustomPathCell& x){
    text.set_text(x.text.text());
    mount.set(x.mount);
    move_forward.set(x.move_forward.current_text());
    move_speed.set(x.move_speed);
    left_x.set(x.left_x);
    left_y.set(x.left_y);
    jump_wait.set(x.jump_wait.current_text());
    wait.set(x.wait.current_text());
}
CustomPathCell::CustomPathCell(EnumDropdownCell<PathAction>& action)
    : BatchOption(LockMode::LOCK_WHILE_RUNNING, true)
    , m_action(action)
    , text("", false)
    , mount(PathMount_Database(), LockMode::LOCK_WHILE_RUNNING, PathMount::NO_MOUNT)
    , move_forward(
        "Duration (ms):", false,
        LockMode::LOCK_WHILE_RUNNING,
        0ms, Milliseconds::max(),
        "2000 ms"
    )
    , move_speed(PathSpeed_Database(), LockMode::LOCK_WHILE_RUNNING,PathSpeed::NORMAL_SPEED)
    , left_x("x: [left: -1.0, right: 1.0]", LockMode::LOCK_WHILE_RUNNING, 0, -1.0, 1.0)
    , left_y("y: [backward: -1.0, forward: 1.0]", LockMode::LOCK_WHILE_RUNNING, 0, -1.0, 1.0)
    , jump_wait(
        "Wait after jump (ms):", false,
        LockMode::LOCK_WHILE_RUNNING,
        0ms, Milliseconds::max(),
        "2000 ms"
    )
    , wait(
        "Wait Time (ms):", false,
        LockMode::LOCK_WHILE_RUNNING,
        0ms, Milliseconds::max(),
        "2000 ms"
    )
{
    PA_ADD_STATIC(text);
    PA_ADD_OPTION(mount);
    PA_ADD_OPTION(move_forward);
    PA_ADD_OPTION(move_speed);
    PA_ADD_OPTION(left_x);
    PA_ADD_OPTION(left_y);
    PA_ADD_OPTION(jump_wait);
    PA_ADD_OPTION(wait);

    CustomPathCell::on_config_value_changed(this);
    action.add_listener(*this);
}
void CustomPathCell::on_config_value_changed(void* object){
    text.set_visibility(ConfigOptionState::HIDDEN);
    mount.set_visibility(ConfigOptionState::HIDDEN);
    move_forward.set_visibility(ConfigOptionState::HIDDEN);
    move_speed.set_visibility(ConfigOptionState::HIDDEN);
    left_x.set_visibility(ConfigOptionState::HIDDEN);
    left_y.set_visibility(ConfigOptionState::HIDDEN);
    jump_wait.set_visibility(ConfigOptionState::HIDDEN);
    wait.set_visibility(ConfigOptionState::HIDDEN);
    switch (m_action){
    case PathAction::NO_ACTION:
        break;
    case PathAction::CHANGE_MOUNT:
        mount.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::MOVE_FORWARD:
        move_forward.set_visibility(ConfigOptionState::ENABLED);
        move_speed.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::MOVE_IN_DIRECTION:
        move_forward.set_visibility(ConfigOptionState::ENABLED);
        left_x.set_visibility(ConfigOptionState::ENABLED);
        left_y.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::CENTER_CAMERA:
        text.set_text("Center the camera so that you can move straight forward.");
        text.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::JUMP:
        jump_wait.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::WAIT:
        wait.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::START_LISTEN:
        text.set_text("If shiny detected, use \"Destination Shiny Action\".");
        text.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::END_LISTEN:
        text.set_text("If shiny detected, use \"Enroute Shiny Action\".");
        text.set_visibility(ConfigOptionState::ENABLED);
        break;
    default:
        break;
    }
}




CustomPathTableRow::CustomPathTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , action(PathAction_Database(), LockMode::LOCK_WHILE_RUNNING, PathAction::NO_ACTION)
    , parameters(action)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(parameters);
}
std::unique_ptr<EditableTableRow> CustomPathTableRow::clone() const{
    std::unique_ptr<CustomPathTableRow> ret(new CustomPathTableRow(parent()));
    ret->action.set(action);
    ret->parameters = parameters;
    return ret;
}
void CustomPathTableRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    const JsonValue* value = obj->get_value("Action");
    if (value == nullptr){
        return;
    }
    action.load_json(*value);

    switch(action){
    case PathAction::CHANGE_MOUNT:
        value = obj->get_value("Mount");
        if (value != nullptr){
            parameters.mount.load_json(*value);
        }
        break;
    case PathAction::MOVE_FORWARD:
        value = obj->get_value("MoveForwardTicks");
        if (value != nullptr && value->is_integer()){
            parameters.move_forward.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("MoveForwardMs");
        if (value != nullptr){
            parameters.move_forward.load_json(*value);
        }
        value = obj->get_value("Speed");
        if (value != nullptr){
            parameters.move_speed.load_json(*value);
        }
        break;
    case PathAction::MOVE_IN_DIRECTION:
        value = obj->get_value("MoveForwardTicks");
        if (value != nullptr && value->is_integer()){
            parameters.move_forward.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("MoveForwardMs");
        if (value != nullptr){
            parameters.move_forward.load_json(*value);
        }
//        value = obj->get_value("Speed");
//        if (value != nullptr){
//            parameters.move_speed.load_json(*value);
//        }
        value = obj->get_value("MoveDirectionX");
        if (value != nullptr){
            parameters.left_x.load_json(*value);
        }
        value = obj->get_value("MoveDirectionY");
        if (value != nullptr){
            parameters.left_y.load_json(*value);
        }
        break;
    case PathAction::JUMP:
        value = obj->get_value("JumpWaitTicks");
        if (value != nullptr && value->is_integer()){
            parameters.jump_wait.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("JumpWaitMs");
        if (value != nullptr){
            parameters.jump_wait.load_json(*value);
        }
        break;
    case PathAction::WAIT:
        value = obj->get_value("WaitTicks");
        if (value != nullptr && value->is_integer()){
            parameters.wait.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("WaitMs");
        if (value != nullptr){
            parameters.wait.load_json(*value);
        }
        break;
    default:
        break;
    }
}
JsonValue CustomPathTableRow::to_json() const{
    JsonObject obj;
    obj["Action"] = action.to_json();
    switch (action){
    case PathAction::CHANGE_MOUNT:
        obj["Mount"] = parameters.mount.to_json();
        break;
    case PathAction::MOVE_FORWARD:
        obj["MoveForwardMs"] = parameters.move_forward.to_json();
        obj["Speed"] = parameters.move_speed.to_json();
        break;
    case PathAction::MOVE_IN_DIRECTION:
        obj["MoveForwardMs"] = parameters.move_forward.to_json();
//        obj["Speed"] = parameters.move_speed.to_json();
        obj["MoveDirectionX"] = parameters.left_x.to_json();
        obj["MoveDirectionY"] = parameters.left_y.to_json();
        break;
    case PathAction::JUMP:
        obj["JumpWaitMs"] = parameters.jump_wait.to_json();
        break;
    case PathAction::WAIT:
        obj["WaitMs"] = parameters.wait.to_json();
        break;
    default:
        break;
    }
    return obj;
}

CustomPathTable::CustomPathTable()
    : EditableTableOption_t<CustomPathTableRow>(
        "<b>Custom Path Table:</b><br>"
        "Set a sequence of actions to navigate the map. By default, the shiny detected behavior is \"Enroute Shiny Action\".<br>"
        "<font color=\"red\">If you wish to ignore enroute shinies, make sure you set \"Enroute Shiny Action\" to ignore shinies.</font>",
        LockMode::LOCK_WHILE_RUNNING,
        false,  //  Disable the save/load buttons since we have our own.
        make_defaults()
    )
{}
std::vector<std::string> CustomPathTable::make_header() const{
    return std::vector<std::string>{
        "Action",
        "Parameters",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> CustomPathTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    auto row = std::make_unique<CustomPathTableRow>(*this);
    row->action.set(PathAction::START_LISTEN);
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>(*this);
    row->action.set(PathAction::CHANGE_MOUNT);
    row->parameters.mount.set(PathMount::WYRDEER);
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>(*this);
    row->action.set(PathAction::MOVE_IN_DIRECTION);
    row->parameters.move_forward.set("3200 ms");
    row->parameters.left_x.set(-1.0);
    row->parameters.left_y.set(1.0);
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>(*this);
    row->action.set(PathAction::CENTER_CAMERA);
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>(*this);
    row->action.set(PathAction::MOVE_FORWARD);
    row->parameters.move_speed.set(PathSpeed::DASH);
    row->parameters.move_forward.set("3200 ms");
    ret.emplace_back(std::move(row));

    return ret;
}









CustomPathTableFromJubilife::CustomPathTableFromJubilife()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
//    : PATH(
//        "<b>Custom Path Table:</b><br>"
//        "Set a sequence of actions to navigate the map. By default, the shiny detected behavior is \"Enroute Shiny Action\".<br>"
//        "<font color=\"red\">If you wish to ignore enroute shinies, make sure you set \"Enroute Shiny Action\" to ignore shinies.</font>",
//        m_factory, make_defaults()
//    )
{
    PA_ADD_OPTION(TRAVEL_LOCATION);
    PA_ADD_OPTION(PATH);
}


class CustomPathTableWidget : public QWidget, public ConfigWidget{
public:
    CustomPathTableWidget(QWidget& parent, CustomPathTableFromJubilife& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
    {
        // m_table_widget is class EditableTableWidget : public EditableTableBaseWidget, public ConfigWidget
        // EditableTableBaseWidget inherits QWidget.
        // Since it's a QWidget, we don't need to care about its memory ownership after its parent is set (as `this`).

        m_travel_location = value.TRAVEL_LOCATION.make_QtWidget(*this);
        m_table_widget = value.PATH.make_QtWidget(*this);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(&m_travel_location->widget());
        layout->addWidget(&m_table_widget->widget());

        QHBoxLayout* button_layout = new QHBoxLayout();
        button_layout->setContentsMargins(0, 0, 0, 0);
        layout->addLayout(button_layout);
        auto load_button = new QPushButton("Load Path", this);
        button_layout->addWidget(load_button, 1);
        auto save_button = new QPushButton("Save Path", this);
        button_layout->addWidget(save_button, 1);
        button_layout->addStretch(2);

        connect(load_button,  &QPushButton::clicked, this, [&value, this](bool){
            std::string path = QFileDialog::getOpenFileName(this, tr("Open option file"), ".", "*.json").toStdString();
            std::cout << "Load CustomPathTableFromJubilife from " << path << std::endl;
            if (path.empty()){
                return;
            }
            JsonValue json = load_json_file(path);
            JsonObject& root = json.to_object_throw(path);
            JsonValue& obj = root.get_value_throw("CUSTOM_PATH_TABLE", path);
            value.load_json(obj);
            if (m_table_widget == nullptr){
                QMessageBox box;
                box.critical(nullptr, "Error", "Internal code error, cannot convert to EditableTableBaseWidget.");
                return;
            }
//            m_travel_location->update();
//            m_table_widget->update();
        });

        connect(save_button,  &QPushButton::clicked, this, [&value, this](bool){
            std::string path = QFileDialog::getSaveFileName(this, tr("Open option file"), ".", "*.json").toStdString();
            std::cout << "Save CustomPathTableFromJubilife from " << path << std::endl;
            if (path.size() > 0){
                try{
                    JsonObject root;
                    root["CUSTOM_PATH_TABLE"] = value.to_json();
                    root.dump(path);
                }catch (FileException&){
                    QMessageBox box;
                    box.critical(nullptr, "Error", QString::fromStdString("Failed to save to file: " + path));
                    return;
                }
            }
        });
    }

private:
    ConfigWidget* m_travel_location = nullptr;
    ConfigWidget* m_table_widget = nullptr;
};

ConfigWidget* CustomPathTableFromJubilife::make_QtWidget(QWidget& parent){
    return new CustomPathTableWidget(parent, *this);
}


}
}
}
