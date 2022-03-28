/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>

#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Options/Pokemon_IVCheckerWidget.h"
#include "CommonFramework/Options/EditableTableOption-EnumTableCell.h"
#include "PokemonLA_CustomPathTable.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const QString PathAction_NAMES[] = {
    "NO Action",
    "Change Mount",
    "Rotate Camera",
    "Move Forward",
    "Jump",
    "Wait",
};

const std::map<QString, PathAction> PathAction_MAP{
    {PathAction_NAMES[0], PathAction::NO_ACTION},
    {PathAction_NAMES[1], PathAction::CHANGE_MOUNT},
    {PathAction_NAMES[2], PathAction::ROTATE_CAMERA},
    {PathAction_NAMES[3], PathAction::MOVE_FORWARD},
    {PathAction_NAMES[4], PathAction::JUMP},
    {PathAction_NAMES[5], PathAction::WAIT},
};


const QString PathMount_NAMES[] = {
    "No Mount",
    "Wrydeer",
    "Ursaluna",
    "Basculegion",
    "Sneasler",
    "Braviary",
};

const std::map<QString, PathMount> PathMount_MAP{
    {PathMount_NAMES[0], PathMount::NO_MOUNT},
    {PathMount_NAMES[1], PathMount::WYRDEER},
    {PathMount_NAMES[2], PathMount::URSALUNA},
    {PathMount_NAMES[3], PathMount::BASCULEGION},
    {PathMount_NAMES[4], PathMount::SNEASLER},
    {PathMount_NAMES[5], PathMount::BRAVIARY},
};


const QString PathSpeed_NAMES[] = {
    "Normal Speed",
    "Slow Speed",
    "Run",
    "Dash",
    "Dash (Braviary B Spam)",
    "Dive",
};

const std::map<QString, PathSpeed> PathSpeed_MAP{
    {PathSpeed_NAMES[0], PathSpeed::NORMAL_SPEED},
    {PathSpeed_NAMES[1], PathSpeed::SLOW_SPEED},
    {PathSpeed_NAMES[2], PathSpeed::RUN},
    {PathSpeed_NAMES[3], PathSpeed::DASH},
    {PathSpeed_NAMES[4], PathSpeed::DASH_B_SPAM},
    {PathSpeed_NAMES[5], PathSpeed::DIVE},
};


    // switch(action){
    //     case PathAction::CHANGE_MOUNT:
    //         break;
    //     case PathAction::ROTATE_CAMERA:
    //         break;
    //     case PathAction::MOVE_FORWARD:
    //         break;
    //     case PathAction::JUMP:
    //         break;
    //     case PathAction::WAIT:
    //         break;
    //     default:
    //         break;
    // }


CustomPathTableRow::CustomPathTableRow() {}

void CustomPathTableRow::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    {
        QString value;
        if (json_get_string(value, obj, "Action")){
            const auto iter = PathAction_MAP.find(value);
            if (iter != PathAction_MAP.end()){
                action = iter->second;
            }
        }
        switch(action){
        case PathAction::CHANGE_MOUNT:
            if (json_get_string(value, obj, "Mount")){
                const auto iter = PathMount_MAP.find(value);
                if (iter != PathMount_MAP.end()){
                    mount = iter->second;
                }
            }
            break;
        case PathAction::ROTATE_CAMERA:
            json_get_int(camera_turn_ticks, obj, "CameraTurnTicks");
            break;
        case PathAction::MOVE_FORWARD:
            json_get_int(move_forward_ticks, obj, "MoveForwardTicks");
            if (json_get_string(value, obj, "Speed")){
                const auto iter = PathSpeed_MAP.find(value);
                if (iter != PathSpeed_MAP.end()){
                    move_speed = iter->second;
                }
            }
            break;
        case PathAction::JUMP:
            json_get_int(jump_wait_ticks, obj, "JumpWaitTicks");
            break;
        case PathAction::WAIT:
            json_get_int(wait_ticks, obj, "WaitTicks");
            break;
        default:
            break;
        }
    }

    // json_get_bool(switch_pokemon, obj, "Switch");
    // json_get_int(num_turns_to_switch, obj, "Turns", 0);
}

QJsonValue CustomPathTableRow::to_json() const{
    QJsonObject obj;
    obj.insert("Action", PathAction_NAMES[(size_t)action]);
    switch(action){
    case PathAction::CHANGE_MOUNT:
        obj.insert("Mount", PathMount_NAMES[(size_t)mount]);
        break;
    case PathAction::ROTATE_CAMERA:
        obj.insert("CameraTurnTicks", camera_turn_ticks);
        break;
    case PathAction::MOVE_FORWARD:
        obj.insert("MoveForwardTicks", move_forward_ticks);
        obj.insert("Speed", PathMount_NAMES[(size_t)move_speed]);
        break;
    case PathAction::JUMP:
        obj.insert("JumpWaitTicks", jump_wait_ticks);
        break;
    case PathAction::WAIT:
        obj.insert("WaitTicks", wait_ticks);
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
};

ActionParameterWidget::ActionParameterWidget(QWidget& parent, CustomPathTableRow& row): QWidget(&parent), m_row(row)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    // Widget 0
    m_layout->addWidget(make_enum_table_cell(*this, PathMount_MAP.size(), PathMount_NAMES, m_row.mount));

    // Widget 1, 2
    m_layout->addWidget(new QLabel("Ticks to turn camera (right: +, left: -):"));
    m_layout->addWidget(make_number_table_cell(*this, m_row.camera_turn_ticks));

    // Widget 3, 4, 5, 6
    m_layout->addWidget(new QLabel("Ticks to move forward:"));
    m_layout->addWidget(make_number_table_cell(*this, m_row.move_forward_ticks));
    m_layout->addWidget(new QLabel(" "));
    m_layout->addWidget(make_enum_table_cell(*this, PathSpeed_MAP.size(), PathSpeed_NAMES, m_row.move_speed));

    // Widget 7, 8
    m_layout->addWidget(new QLabel("Ticks after jump:"));
    m_layout->addWidget(make_number_table_cell(*this, m_row.jump_wait_ticks));

    // Widget 9, 10
    m_layout->addWidget(new QLabel("Ticks"));
    m_layout->addWidget(make_number_table_cell(*this, m_row.wait_ticks));

    setParameter();
}

void ActionParameterWidget::setParameter(){
    for(int i = 0; i < m_layout->count(); i++){
        m_layout->itemAt(i)->widget()->hide();
    }
    switch(m_row.action){
        case PathAction::CHANGE_MOUNT:
            m_layout->itemAt(0)->widget()->show();
            break;
        case PathAction::ROTATE_CAMERA:
            m_layout->itemAt(1)->widget()->show();
            m_layout->itemAt(2)->widget()->show();
            break;
        case PathAction::MOVE_FORWARD:
            for(int i = 3; i < 7; i++){
                m_layout->itemAt(i)->widget()->show();
            }
            break;
        case PathAction::JUMP:
            m_layout->itemAt(7)->widget()->show();
            m_layout->itemAt(8)->widget()->show();
            break;
        case PathAction::WAIT:
            m_layout->itemAt(9)->widget()->show();
            m_layout->itemAt(10)->widget()->show();
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
        box->addItem(PathAction_NAMES[i]);
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




std::vector<std::unique_ptr<EditableTableRow>> CustomPathTableTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::unique_ptr<CustomPathTableRow>(new CustomPathTableRow()));
    return ret;
}

CustomPathTableTable::CustomPathTableTable()
    : m_table(
        "<b>Custom Path Table:</b><br>"
        "Set a sequence of actions to navigate the map. ",
        m_factory, true, make_defaults()
    )
{}

void CustomPathTableTable::load_json(const QJsonValue& json){
    m_table.load_json(json);
}

QJsonValue CustomPathTableTable::to_json() const{
    return m_table.to_json();
}

void CustomPathTableTable::restore_defaults(){
    m_table.restore_defaults();
}

ConfigWidget* CustomPathTableTable::make_ui(QWidget& parent){
    return m_table.make_ui(parent);
}


}
}
}
