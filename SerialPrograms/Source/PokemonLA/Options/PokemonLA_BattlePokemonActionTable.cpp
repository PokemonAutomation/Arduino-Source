/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Options/Pokemon_IVCheckerWidget.h"
#include "PokemonLA_BattlePokemonActionTable.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const QString MoveStyle_NAMES[] = {
    "No Style",
    "Agile",
    "Strong",
};
const std::map<QString, MoveStyle> MoveStyle_MAP{
    {MoveStyle_NAMES[0], MoveStyle::NoStyle},
    {MoveStyle_NAMES[1], MoveStyle::Agile},
    {MoveStyle_NAMES[2], MoveStyle::Strong},
};


BattlePokemonActionRow::BattlePokemonActionRow() {}

void BattlePokemonActionRow::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    {
        QString value;
        for(int i = 0; i < 4; i++){
            if (json_get_string(value, obj, "Style" + QString::number(i))){
                const auto iter = MoveStyle_MAP.find(value);
                if (iter != MoveStyle_MAP.end()){
                    style[i] = iter->second;
                }
            }
        }
    }

    json_get_bool(switch_pokemon, obj, "Switch");
    json_get_int(num_turns_to_switch, obj, "Turns", 0);
}

QJsonValue BattlePokemonActionRow::to_json() const{
    QJsonObject obj;
    for(int i = 0; i < 4; i++){
        obj.insert("Style"+QString::number(i), MoveStyle_NAMES[(size_t)style[i]]);
    }
    
    obj.insert("Switch", switch_pokemon);
    obj.insert("Turns", num_turns_to_switch);
    return obj;
}

std::unique_ptr<EditableTableRow> BattlePokemonActionRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new BattlePokemonActionRow(*this));
}

std::vector<QWidget*> BattlePokemonActionRow::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    for(int i = 0; i < 4; i++){
        widgets.emplace_back(make_style_box(parent, style[i]));
    }
    widgets.emplace_back(make_switch_box(parent));
    widgets.emplace_back(make_turns_box(parent));
    return widgets;
}

QWidget* BattlePokemonActionRow::make_style_box(QWidget& parent, MoveStyle& style){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem(MoveStyle_NAMES[0]);
    box->addItem(MoveStyle_NAMES[1]);
    box->addItem(MoveStyle_NAMES[2]);
    box->setCurrentIndex((int)style);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            style = (MoveStyle)index;
        }
    );
    return box;
}

QWidget* BattlePokemonActionRow::make_switch_box(QWidget& parent){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(this->switch_pokemon);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            this->switch_pokemon = box->isChecked();
        }
    );
    return widget;
}

QWidget* BattlePokemonActionRow::make_turns_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(QString::number(num_turns_to_switch), &parent);
    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& text){
            bool ok = false;
            const int current = (int)text.toLong(&ok);
            QPalette palette;
            if (ok && current >= 0){
                this->num_turns_to_switch = current;
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            box->setPalette(palette);
        }
    );
    box->connect(
        box, &QLineEdit::editingFinished,
        box, [=](){
            box->setText(QString::number(this->num_turns_to_switch));
        }
    );
    return box;
}




QStringList BattlePokemonActionTableFactory::make_header() const{
    QStringList list;
    list << "Move 1 Style" << "Move 2 Style" << "Move 3 Style" << "Move 4 Style" << "Switch " + STRING_POKEMON << "Num Turns to Switch";
    return list;
}

std::unique_ptr<EditableTableRow> BattlePokemonActionTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new BattlePokemonActionRow());
}




std::vector<std::unique_ptr<EditableTableRow>> BattlePokemonActionTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::unique_ptr<BattlePokemonActionRow>(new BattlePokemonActionRow()));
    return ret;
}

BattlePokemonActionTable::BattlePokemonActionTable()
    : m_table(
        "<b>" + STRING_POKEMON + " Action Table:</b><br>"
        "Set what move styles to use and whether to switch the " + STRING_POKEMON + " after some turns. "
        "Each row is the action for one " + STRING_POKEMON + ". "
        "The table follows the order that " + STRING_POKEMON + " are sent to battle.",
        m_factory, true, make_defaults()
    )
{}

void BattlePokemonActionTable::load_json(const QJsonValue& json){
    m_table.load_json(json);
}

QJsonValue BattlePokemonActionTable::to_json() const{
    return m_table.to_json();
}

void BattlePokemonActionTable::restore_defaults(){
    m_table.restore_defaults();
}

ConfigWidget* BattlePokemonActionTable::make_ui(QWidget& parent){
    return m_table.make_ui(parent);
}

MoveStyle BattlePokemonActionTable::get_style(size_t pokemon, size_t move){
    if (pokemon >= m_table.size()){
        return MoveStyle::NoStyle;
    }

    const BattlePokemonActionRow& action = static_cast<const BattlePokemonActionRow&>(m_table[pokemon]);
    return action.style[move];
}

bool BattlePokemonActionTable::switch_pokemon(size_t pokemon, size_t num_turns){
    if (pokemon >= m_table.size()){
        return false;
    }

    const BattlePokemonActionRow& action = static_cast<const BattlePokemonActionRow&>(m_table[pokemon]);
    return action.switch_pokemon && num_turns >= (size_t)action.num_turns_to_switch;
}












}
}
}
