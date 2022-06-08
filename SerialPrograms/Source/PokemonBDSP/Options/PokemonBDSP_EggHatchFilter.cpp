/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QComboBox>
#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "Pokemon/Options/Pokemon_IVCheckerWidget.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "PokemonBDSP_EggHatchFilter.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const QString EggHatchAction_NAMES[] = {
    "Stop Program",
    "Keep",
};
const std::map<QString, EggHatchAction> EncounterAction_MAP{
    {EggHatchAction_NAMES[0], EggHatchAction::StopProgram},
    {EggHatchAction_NAMES[1], EggHatchAction::Keep},
};


const QString EggHatchShinyFilter_NAMES[] = {
    "Anything",
    "Not Shiny",
    "Shiny",
};
const std::map<QString, EggHatchShinyFilter> ShinyFilter_MAP{
    {EggHatchShinyFilter_NAMES[0], EggHatchShinyFilter::Anything},
    {EggHatchShinyFilter_NAMES[1], EggHatchShinyFilter::NotShiny},
    {EggHatchShinyFilter_NAMES[2], EggHatchShinyFilter::Shiny},
};

const QString EggHatchGenderFilter_NAMES[] = {
    "Any",
    "Male",
    "Female",
    "Genderless",
};
const std::map<QString, EggHatchGenderFilter> GenderFilter_MAP{
    {EggHatchGenderFilter_NAMES[0], EggHatchGenderFilter::Any},
    {EggHatchGenderFilter_NAMES[1], EggHatchGenderFilter::Male},
    {EggHatchGenderFilter_NAMES[2], EggHatchGenderFilter::Female},
    {EggHatchGenderFilter_NAMES[3], EggHatchGenderFilter::Genderless}
};



EggHatchFilterRow::EggHatchFilterRow(EggHatchShinyFilter p_shiny)
    : shiny(p_shiny)
{}
void EggHatchFilterRow::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    {
        QString value;
        if (json_get_string(value, obj, "Action")){
            auto iter = EncounterAction_MAP.find(value);
            if (iter != EncounterAction_MAP.end()){
                action = iter->second;
            }
        }
    }
    {
        QString value;
        if (json_get_string(value, obj, "Shininess")){
            auto iter = ShinyFilter_MAP.find(value);
            if (iter != ShinyFilter_MAP.end()){
                shiny = iter->second;
            }
        }
    }
    iv_hp = IVCheckerFilter_string_to_enum(json_get_string_nothrow(obj, "IV-HP"));
    iv_atk = IVCheckerFilter_string_to_enum(json_get_string_nothrow(obj, "IV-Atk"));
    iv_def = IVCheckerFilter_string_to_enum(json_get_string_nothrow(obj, "IV-Def"));
    iv_spatk = IVCheckerFilter_string_to_enum(json_get_string_nothrow(obj, "IV-SpAtk"));
    iv_spdef = IVCheckerFilter_string_to_enum(json_get_string_nothrow(obj, "IV-SpDef"));
    iv_speed = IVCheckerFilter_string_to_enum(json_get_string_nothrow(obj, "IV-Speed"));
}
QJsonValue EggHatchFilterRow::to_json() const{
    QJsonObject obj;
    obj.insert("Action", EggHatchAction_NAMES[(size_t)action]);
    obj.insert("Shininess", EggHatchShinyFilter_NAMES[(size_t)shiny]);
    obj.insert("IV-HP", IVCheckerFilter_enum_to_string(iv_hp));
    obj.insert("IV-Atk", IVCheckerFilter_enum_to_string(iv_atk));
    obj.insert("IV-Def", IVCheckerFilter_enum_to_string(iv_def));
    obj.insert("IV-SpAtk", IVCheckerFilter_enum_to_string(iv_spatk));
    obj.insert("IV-SpDef", IVCheckerFilter_enum_to_string(iv_spdef));
    obj.insert("IV-Speed", IVCheckerFilter_enum_to_string(iv_speed));
    return obj;
}
std::unique_ptr<EditableTableRow> EggHatchFilterRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new EggHatchFilterRow(*this));
}
std::vector<QWidget*> EggHatchFilterRow::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    widgets.emplace_back(make_action_box(parent));
    widgets.emplace_back(make_shiny_box(parent));
    widgets.emplace_back(make_iv_box(parent, iv_hp));
    widgets.emplace_back(make_iv_box(parent, iv_atk));
    widgets.emplace_back(make_iv_box(parent, iv_def));
    widgets.emplace_back(make_iv_box(parent, iv_spatk));
    widgets.emplace_back(make_iv_box(parent, iv_spdef));
    widgets.emplace_back(make_iv_box(parent, iv_speed));
    widgets.emplace_back(make_gender_box(parent));
    return widgets;
}
QWidget* EggHatchFilterRow::make_action_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem(EggHatchAction_NAMES[0]);
    box->addItem(EggHatchAction_NAMES[1]);
    box->setCurrentIndex((int)action);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            action = (EggHatchAction)index;
        }
    );
    return box;
}
QWidget* EggHatchFilterRow::make_shiny_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem(EggHatchShinyFilter_NAMES[0]);
    box->addItem(EggHatchShinyFilter_NAMES[1]);
    box->addItem(EggHatchShinyFilter_NAMES[2]);
    box->setCurrentIndex((int)shiny);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            shiny = (EggHatchShinyFilter)index;
        }
    );
    return box;
}
QWidget* EggHatchFilterRow::make_iv_box(QWidget& parent, IVCheckerFilter& iv){
    IVCheckerFilterWidget* box = new IVCheckerFilterWidget(parent, iv);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&, box](int index){
            iv = *box;
        }
    );
    return box;
}

QWidget* EggHatchFilterRow::make_gender_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem(EggHatchGenderFilter_NAMES[0]);
    box->addItem(EggHatchGenderFilter_NAMES[1]);
    box->addItem(EggHatchGenderFilter_NAMES[2]);
    box->addItem(EggHatchGenderFilter_NAMES[3]);
    box->setCurrentIndex((int)shiny);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            shiny = (EggHatchShinyFilter)index;
        }
    );
    return box;
}


QStringList EggHatchFilterOptionFactory::make_header() const{
    QStringList list;
    list << "Action" << "Shininess" << "HP" << "Attack" << "Defense" << "Sp. Attack" << "Sp. Defense" << "Speed" << "Gender";
    return list;
}
std::unique_ptr<EditableTableRow> EggHatchFilterOptionFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new EggHatchFilterRow());
}



std::vector<std::unique_ptr<EditableTableRow>> EggHatchFilterOption::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::unique_ptr<EggHatchFilterRow>(new EggHatchFilterRow(EggHatchShinyFilter::Shiny)));
    return ret;
}

EggHatchFilterOption::EggHatchFilterOption()
    : m_table(
        "<b>Actions Table:</b><br>"
        "If a hatchling matches one of these filters, the specified action will be performed. "
        "Otherwise, it will be released. "
        "If multiple entries apply and have conflicting actions, the program will stop.<br>"
        "IV checking requires that your right panel be set to the IV Judge.",
        m_factory, make_defaults()
    )
{}

void EggHatchFilterOption::load_json(const QJsonValue& json){
    m_table.load_json(json);
}
QJsonValue EggHatchFilterOption::to_json() const{
    return m_table.to_json();
}
void EggHatchFilterOption::restore_defaults(){
    m_table.restore_defaults();
}
ConfigWidget* EggHatchFilterOption::make_ui(QWidget& parent){
    return m_table.make_ui(parent);
}

EggHatchAction EggHatchFilterOption::get_action(bool shiny, const IVCheckerReader::Results& IVs, EggHatchGenderFilter gender) const{
    EggHatchAction action = EggHatchAction::Release;
    for (size_t c = 0; c < m_table.size(); c++){
        const EggHatchFilterRow& filter = static_cast<const EggHatchFilterRow&>(m_table[c]);

        //  Check the shiny filter.
        switch (filter.shiny){
        case EggHatchShinyFilter::Anything:
            break;
        case EggHatchShinyFilter::NotShiny:
            if (shiny){
                continue;
            }
            break;
        case EggHatchShinyFilter::Shiny:
            if (!shiny){
                continue;
            }
            break;
        }

        //  Check all the IV filters.
        if (!IVChecker_filter_match(filter.iv_hp, IVs.hp)) continue;
        if (!IVChecker_filter_match(filter.iv_atk, IVs.attack)) continue;
        if (!IVChecker_filter_match(filter.iv_def, IVs.defense)) continue;
        if (!IVChecker_filter_match(filter.iv_spatk, IVs.spatk)) continue;
        if (!IVChecker_filter_match(filter.iv_spdef, IVs.spdef)) continue;
        if (!IVChecker_filter_match(filter.iv_speed, IVs.speed)) continue;

//        if(filter.gender == EggHatchGenderFilter::Any) continue;
//        if (filter.gender ==  gender) continue;

        //  No action matched so far. Take the current action and continue.
        if (action == EggHatchAction::Release){
            action = filter.action;
            continue;
        }

        //  Conflicting actions.
        if (action != filter.action){
            global_logger_tagged().log("Multiple filters matched with conflicting actions. Stopping program...", COLOR_RED);
            return EggHatchAction::StopProgram;
        }
    }
    return action;
}













}
}
}
