/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Options/EditableTableOption.h"
#include "CommonFramework/Options/EditableTableWidget.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/EditableTableOption-EnumTableCell.h"
#include "Pokemon/Options/Pokemon_IVCheckerWidget.h"
#include "PokemonLA_CustomPathTable.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const std::string PathAction_NAMES[] = {
    "NO Action",
    "Change Mount",
//    "Rotate Camera",
    "Move Forward",
    "Move in Direction",
    "Center Camera",
    "Jump",
    "Wait",
    "Start Listen",
    "End Listen",
};

const std::map<std::string, PathAction> PathAction_MAP{
    {PathAction_NAMES[0], PathAction::NO_ACTION},
    {PathAction_NAMES[1], PathAction::CHANGE_MOUNT},
//    {PathAction_NAMES[2], PathAction::ROTATE_CAMERA},
    {PathAction_NAMES[2], PathAction::MOVE_FORWARD},
    {PathAction_NAMES[3], PathAction::MOVE_IN_DIRECTION},
    {PathAction_NAMES[4], PathAction::CENTER_CAMERA},
    {PathAction_NAMES[5], PathAction::JUMP},
    {PathAction_NAMES[6], PathAction::WAIT},
    {PathAction_NAMES[7], PathAction::START_LISTEN},
    {PathAction_NAMES[8], PathAction::END_LISTEN},
};


const std::string PathMount_NAMES[] = {
    "No Mount",
    "Wrydeer",
    "Ursaluna",
    "Basculegion",
    "Sneasler",
    "Braviary",
};

const std::map<std::string, PathMount> PathMount_MAP{
    {PathMount_NAMES[0], PathMount::NO_MOUNT},
    {PathMount_NAMES[1], PathMount::WYRDEER},
    {PathMount_NAMES[2], PathMount::URSALUNA},
    {PathMount_NAMES[3], PathMount::BASCULEGION},
    {PathMount_NAMES[4], PathMount::SNEASLER},
    {PathMount_NAMES[5], PathMount::BRAVIARY},
};


const std::string PathSpeed_NAMES[] = {
    "Normal Speed",
    "Slow Speed",
    "Run on Foot",
    "Dash on Ride",
    "Dash on Braviary B Spam",
    "Dive on Braviary",
};

const std::map<std::string, PathSpeed> PathSpeed_MAP{
    {PathSpeed_NAMES[0], PathSpeed::NORMAL_SPEED},
    {PathSpeed_NAMES[1], PathSpeed::SLOW_SPEED},
    {PathSpeed_NAMES[2], PathSpeed::RUN},
    {PathSpeed_NAMES[3], PathSpeed::DASH},
    {PathSpeed_NAMES[4], PathSpeed::DASH_B_SPAM},
    {PathSpeed_NAMES[5], PathSpeed::DIVE},
};



CustomPathTableRow::CustomPathTableRow() {}

void CustomPathTableRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }

    const std::string* str = obj->get_string("Action");
    if (str != nullptr){
        const auto iter = PathAction_MAP.find(*str);
        if (iter != PathAction_MAP.end()){
            action = iter->second;
        }
    }

    switch(action){
    case PathAction::CHANGE_MOUNT:
        str = obj->get_string("Mount");
        if (str != nullptr){
            const auto iter = PathMount_MAP.find(*str);
            if (iter != PathMount_MAP.end()){
                mount = iter->second;
            }
        }
        break;
    case PathAction::MOVE_FORWARD:
        obj->read_integer(move_forward_ticks, "MoveForwardTicks");
        str = obj->get_string("Speed");
        if (str != nullptr){
            const auto iter = PathSpeed_MAP.find(*str);
            if (iter != PathSpeed_MAP.end()){
                move_speed = iter->second;
            }
        }
        break;
    case PathAction::MOVE_IN_DIRECTION:
        obj->read_integer(move_forward_ticks, "MoveForwardTicks");
        str = obj->get_string("Speed");
        if (str != nullptr){
            const auto iter = PathSpeed_MAP.find(*str);
            if (iter != PathSpeed_MAP.end()){
                move_speed = iter->second;
            }
        }
        obj->read_float(left_x, "MoveDirectionX");
        obj->read_float(left_y, "MoveDirectionY");
        break;
    case PathAction::JUMP:
        obj->read_integer(jump_wait_ticks, "JumpWaitTicks");
        break;
    case PathAction::WAIT:
        obj->read_integer(wait_ticks, "WaitTicks");
        break;
    default:
        break;
    }


    // json_get_bool(switch_pokemon, obj, "Switch");
    // json_get_int(num_turns_to_switch, obj, "Turns", 0);
}

JsonValue CustomPathTableRow::to_json() const{
    JsonObject obj;
    obj["Action"] = PathAction_NAMES[(size_t)action];
    switch(action){
    case PathAction::CHANGE_MOUNT:
        obj["Mount"] = PathMount_NAMES[(size_t)mount];
        break;
#if 0
    case PathAction::ROTATE_CAMERA:
        obj["CameraTurnTicks"] = camera_turn_ticks);
        break;
#endif
    case PathAction::MOVE_FORWARD:
        obj["MoveForwardTicks"] = move_forward_ticks;
        obj["Speed"] = PathSpeed_NAMES[(size_t)move_speed];
        break;
    case PathAction::MOVE_IN_DIRECTION:
        obj["MoveForwardTicks"] = move_forward_ticks;
        obj["Speed"] = PathSpeed_NAMES[(size_t)move_speed];
        obj["MoveDirectionX"] = left_x;
        obj["MoveDirectionY"] = left_y;
        break;
    case PathAction::JUMP:
        obj["JumpWaitTicks"] = jump_wait_ticks;
        break;
    case PathAction::WAIT:
        obj["WaitTicks"] = wait_ticks;
        break;
    default:
        break;
    }
    return obj;
}

std::unique_ptr<EditableTableRow> CustomPathTableRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new CustomPathTableRow(*this));
}

// Each row of the custom path table UI is made by an action dropdown menu and the parameter widget for the chosen
// action. This class is the parameter widget. Because different actions don't share parameters, this class will
// house many different children widgets but only show the ones that related to the current chosen action.
class ActionParameterWidget : public QWidget{
public:
    ActionParameterWidget(QWidget& parent, CustomPathTableRow& row);

    void setParameter();

private:
    CustomPathTableRow& m_row;

    QHBoxLayout* m_layout = nullptr;

    QWidget* m_mounts_dropdown;

    QWidget* m_camera_label;
    QWidget* m_camera_duration;

    QWidget* m_move_label;
    QWidget* m_move_duration;
    QWidget* m_move_space;
    QWidget* m_move_speed;

    QWidget* m_move_x_label;
    QWidget* m_move_x_coord;
    QWidget* m_move_y_label;
    QWidget* m_move_y_coord;

    QWidget* m_center_label;

    QWidget* m_jump_label;
    QWidget* m_jump_duration;

    QWidget* m_wait_label;
    QWidget* m_wait_duration;

    QWidget* m_listen_start;
    QWidget* m_listen_stop;
};

ActionParameterWidget::ActionParameterWidget(QWidget& parent, CustomPathTableRow& row)
    : QWidget(&parent), m_row(row)
{
    //  TODO: Figure out why nesting a widget creates a vertical margin even
    //  when the margins are set to zero.

    this->setContentsMargins(0, 0, 0, 0);
    m_layout = new QHBoxLayout(this);

#if 0
    m_layout->setContentsMargins(0, 0, 0, 0);
    QWidget* widget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    new QHBoxLayout(widget);
     widget->layout()->setContentsMargins(0, 0, 0, 0);
    widget->layout()->addWidget(new QPushButton("test", this));
//    m_layout->addWidget(new QPushButton("test", this));
    m_layout->addWidget(widget);
#endif

#if 1
    m_layout->setContentsMargins(5, 0, 5, 0);

    m_mounts_dropdown = make_enum_table_cell(*this, PathMount_MAP.size(), PathMount_NAMES, m_row.mount);
    m_layout->addWidget(m_mounts_dropdown);

    m_camera_label = new QLabel("Ticks to turn camera (right: +, left: -):", this);
    m_camera_duration = make_integer_table_cell(*this, m_row.camera_turn_ticks);
    m_layout->addWidget(m_camera_label);
    m_layout->addWidget(m_camera_duration);

    m_move_label = new QLabel("Ticks to move:", this);
    m_move_duration = make_integer_table_cell(*this, m_row.move_forward_ticks);
    m_move_space = new QLabel(" ", this);
    m_move_speed = make_enum_table_cell(*this, PathSpeed_MAP.size(), PathSpeed_NAMES, m_row.move_speed);
    m_layout->addWidget(m_move_label);
    m_layout->addWidget(m_move_duration);
    m_layout->addWidget(m_move_space);
    m_layout->addWidget(m_move_speed);

    m_move_x_label = new QLabel("x: [left: -1.0, right: 1.0]", this);
    m_move_x_coord = make_double_table_cell(*this, m_row.left_x, -1.0, 1.0);
    m_move_x_coord->setMaximumWidth(80);
    m_move_y_label = new QLabel("y: [forward: 1.0, backward: -1.0]", this);
    m_move_y_coord = make_double_table_cell(*this, m_row.left_y, -1.0, 1.0);
    m_move_y_coord->setMaximumWidth(80);
    m_layout->addWidget(m_move_x_label);
    m_layout->addWidget(m_move_x_coord);
    m_layout->addWidget(m_move_y_label);
    m_layout->addWidget(m_move_y_coord);

    m_center_label = new QLabel("Center the camera so that you can move straight forward.", this);
    m_layout->addWidget(m_center_label);

    m_jump_label = new QLabel("Ticks after jump:", this);
    m_jump_duration = make_integer_table_cell(*this, m_row.jump_wait_ticks);
    m_layout->addWidget(m_jump_label);
    m_layout->addWidget(m_jump_duration);

    m_wait_label = new QLabel("Ticks", this);
    m_wait_duration = make_integer_table_cell(*this, m_row.wait_ticks);
    m_layout->addWidget(m_wait_label);
    m_layout->addWidget(m_wait_duration);

    m_listen_start = new QLabel("If shiny detected, use \"Destination Shiny Action\".", this);
    m_layout->addWidget(m_listen_start);
    m_listen_stop = new QLabel("If shiny detected, use \"Enroute Shiny Action\".", this);
    m_layout->addWidget(m_listen_stop);

    setParameter();
#endif
}

void ActionParameterWidget::setParameter(){
    for(int i = 0; i < m_layout->count(); i++){
        m_layout->itemAt(i)->widget()->hide();
    }
    switch(m_row.action){
        case PathAction::CHANGE_MOUNT:
            m_mounts_dropdown->show();
            break;
#if 0
        case PathAction::ROTATE_CAMERA:
            m_camera_label->show();
            m_camera_duration->show();
            break;
#endif
        case PathAction::MOVE_FORWARD:
            m_move_label->show();
            m_move_duration->show();
            m_move_space->show();
            m_move_speed->show();
            break;
        case PathAction::MOVE_IN_DIRECTION:
            m_move_label->show();
            m_move_duration->show();
            m_move_x_label->show();
            m_move_x_coord->show();
            m_move_y_label->show();
            m_move_y_coord->show();
            break;
        case PathAction::CENTER_CAMERA:
            m_center_label->show();
            break;
        case PathAction::JUMP:
            m_jump_label->show();
            m_jump_duration->show();
            break;
        case PathAction::WAIT:
            m_wait_label->show();
            m_wait_duration->show();
            break;
        case PathAction::START_LISTEN:
            m_listen_start->show();
            break;
        case PathAction::END_LISTEN:
            m_listen_stop->show();
            break;
        default:
            break;
    }
}


std::vector<QWidget*> CustomPathTableRow::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    auto parameterWidget = new ActionParameterWidget(parent, *this);
    widgets.emplace_back(make_action_box(parent, action, parameterWidget));
    widgets.emplace_back(parameterWidget);
    return widgets;
}

QWidget* CustomPathTableRow::make_action_box(QWidget& parent, PathAction& action, ActionParameterWidget* parameterWidget){
    QComboBox* box = new NoWheelComboBox(&parent);
    for(size_t i = 0; i < PathAction_MAP.size(); i++){
        box->addItem(QString::fromStdString(PathAction_NAMES[i]));
    }
    box->setCurrentIndex((int)action);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&action, parameterWidget](int index){
            if (index < 0){
                index = 0;
            }
            action = (PathAction)index;
            parameterWidget->setParameter();
        }
    );
    return box;
}



QStringList CustomPathTableTableFactory::make_header() const{
    QStringList list;
    list << "Action" << "Parameters";
    return list;
}

std::unique_ptr<EditableTableRow> CustomPathTableTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new CustomPathTableRow());
}




std::vector<std::unique_ptr<EditableTableRow>> CustomPathTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    auto row = std::make_unique<CustomPathTableRow>();
    row->action = PathAction::START_LISTEN;
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>();
    row->action = PathAction::CHANGE_MOUNT;
    row->mount = PathMount::WYRDEER;
    ret.emplace_back(std::move(row));

#if 0
    row = std::make_unique<CustomPathTableRow>();
    row->action = PathAction::ROTATE_CAMERA;
    row->camera_turn_ticks = -100;
    ret.emplace_back(std::move(row));
#endif

    row = std::make_unique<CustomPathTableRow>();
    row->action = PathAction::MOVE_IN_DIRECTION;
    row->move_forward_ticks = 400;
    row->left_x = -1.0;
    row->left_y = 1.0;
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>();
    row->action = PathAction::CENTER_CAMERA;
    ret.emplace_back(std::move(row));

    row = std::make_unique<CustomPathTableRow>();
    row->action = PathAction::MOVE_FORWARD;
    row->move_speed = PathSpeed::DASH;
    row->move_forward_ticks = 400;
    ret.emplace_back(std::move(row));

//    row = std::make_unique<CustomPathTableRow>();
//    row->action = PathAction::END_LISTEN;
//    ret.emplace_back(std::move(row));
    
    return ret;
}

CustomPathTable::CustomPathTable()
    : PATH(
        "<b>Custom Path Table:</b><br>"
        "Set a sequence of actions to navigate the map. By default, the shiny detected behavior is \"Enroute Shiny Action\".<br>"
        "<font color=\"red\">If you wish to ignore enroute shinies, make sure you set \"Enroute Shiny Action\" to ignore shinies.</font>",
        m_factory, make_defaults()
    )
{
    PA_ADD_OPTION(TRAVEL_LOCATION);
    PA_ADD_OPTION(PATH);
}


class CustomPathTableWidget : public QWidget, public ConfigWidget{
public:
    CustomPathTableWidget(QWidget& parent, CustomPathTable& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
    {
        // m_table_widget is class EditableTableWidget : public EditableTableBaseWidget, public ConfigWidget
        // EditableTableBaseWidget inherits QWidget.
        // Since it's a QWidget, we don't need to care about its memory ownership after its parent is set (as `this`).

        m_travel_location = value.TRAVEL_LOCATION.make_ui(*this);
        m_table_widget = static_cast<EditableTableWidget*>(value.PATH.make_ui(*this));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(&m_travel_location->widget());
        layout->addWidget(&m_table_widget->widget());

        QHBoxLayout* button_layout = new QHBoxLayout();
        button_layout->setContentsMargins(0, 0, 0, 0);
        layout->addLayout(button_layout);
        auto load_button = new QPushButton("Load Options", this);
        button_layout->addWidget(load_button);
        auto save_button = new QPushButton("Save Options", this);
        button_layout->addWidget(save_button);

        connect(load_button,  &QPushButton::clicked, this, [&value, this](bool){
            QString path = QFileDialog::getOpenFileName(this, tr("Open option file"), ".", "*.json");
            std::cout << "Load CustomPathTable from " << path.toStdString() << std::endl;
            if (path.size() > 0){
                QJsonDocument doc = read_json_file(path);
                if (!doc.isObject()){
                    QMessageBox box;
                    box.critical(nullptr, "Error", "Invalid option file: " + path + ", no Json object.");
                    return;
                }
                QJsonObject root = doc.object();
                auto it = root.find("CUSTOM_PATH_TABLE");
                if (it == root.end()){
                    QMessageBox box;
                    box.critical(nullptr, "Error", "Invalid option file: " + path + ", no CUSTOM_PATH_TABLE.");
                    return;
                }
                QJsonValue obj = json_get_value_nothrow(root, "CUSTOM_PATH_TABLE");
                value.load_json(from_QJson(obj));
                if (m_table_widget == nullptr){
                    QMessageBox box;
                    box.critical(nullptr, "Error", "Internal code error, cannot convert to EditableTableBaseWidget.");
                    return;
                }
                m_travel_location->update_ui();
                m_table_widget->update_ui();
            }
        });

        connect(save_button,  &QPushButton::clicked, this, [&value, this](bool){
            auto path = QFileDialog::getSaveFileName(this, tr("Open option file"), ".", "*.json");
            std::cout << "Save CustomPathTable from " << path.toStdString() << std::endl;
            if (path.size() > 0){
                try{
                    JsonObject root;
                    root["CUSTOM_PATH_TABLE"] = value.to_json();
                    write_json_file(path, QJsonDocument(to_QJson(std::move(root)).toObject()));
                }catch (FileException&){
                    QMessageBox box;
                    box.critical(nullptr, "Error", "Failed to save to file: " + path);
                    return;
                }
            }
        });
    }

    virtual void restore_defaults() override{
        m_travel_location->restore_defaults();
        m_table_widget->restore_defaults();
    }
    virtual void update_ui() override{
        m_travel_location->update_ui();
        m_table_widget->update_ui();
    }

private:
    ConfigWidget* m_travel_location = nullptr;
    EditableTableWidget* m_table_widget = nullptr;
};

ConfigWidget* CustomPathTable::make_ui(QWidget& parent){
    return new CustomPathTableWidget(parent, *this);
}


}
}
}
