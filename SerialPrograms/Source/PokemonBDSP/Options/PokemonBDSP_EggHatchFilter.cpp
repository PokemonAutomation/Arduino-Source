/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QComboBox>
#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Pokemon/Options/Pokemon_IVCheckerWidget.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "PokemonBDSP_EggHatchFilter.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const std::string EggHatchAction_NAMES[] = {
    "Stop Program",
    "Keep",
};
const std::map<std::string, EggHatchAction> EncounterAction_MAP{
    {EggHatchAction_NAMES[0], EggHatchAction::StopProgram},
    {EggHatchAction_NAMES[1], EggHatchAction::Keep},
};


const std::string EggHatchShinyFilter_NAMES[] = {
    "Anything",
    "Not Shiny",
    "Shiny",
};
const std::map<std::string, EggHatchShinyFilter> ShinyFilter_MAP{
    {EggHatchShinyFilter_NAMES[0], EggHatchShinyFilter::Anything},
    {EggHatchShinyFilter_NAMES[1], EggHatchShinyFilter::NotShiny},
    {EggHatchShinyFilter_NAMES[2], EggHatchShinyFilter::Shiny},
};

const std::string EggHatchGenderFilter_NAMES[] = {
    "Any",
    "Male",
    "Female",
};
const std::map<std::string, EggHatchGenderFilter> GenderFilter_MAP{
    {EggHatchGenderFilter_NAMES[0], EggHatchGenderFilter::Any},
    {EggHatchGenderFilter_NAMES[1], EggHatchGenderFilter::Male},
    {EggHatchGenderFilter_NAMES[2], EggHatchGenderFilter::Female}
};



EggHatchFilterRow::EggHatchFilterRow(EggHatchShinyFilter p_shiny)
    : shiny(p_shiny)
{}
void EggHatchFilterRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const std::string* str;
    {
        str = obj->get_string("Action");
        if (str!= nullptr){
            auto iter = EncounterAction_MAP.find(*str);
            if (iter != EncounterAction_MAP.end()){
                action = iter->second;
            }
        }
    }
    {
        str = obj->get_string("Shininess");
        if (str!= nullptr){
            auto iter = ShinyFilter_MAP.find(*str);
            if (iter != ShinyFilter_MAP.end()){
                shiny = iter->second;
            }
        }
    }
    str = obj->get_string("IV-HP");
    if (str){ iv_hp     = IVCheckerFilter_string_to_enum(*str); }
    str = obj->get_string("IV-Atk");
    if (str){ iv_atk    = IVCheckerFilter_string_to_enum(*str); }
    str = obj->get_string("IV-Def");
    if (str){ iv_def    = IVCheckerFilter_string_to_enum(*str); }
    str = obj->get_string("IV-SpAtk");
    if (str){ iv_spatk  = IVCheckerFilter_string_to_enum(*str); }
    str = obj->get_string("IV-SpDef");
    if (str){ iv_spdef  = IVCheckerFilter_string_to_enum(*str); }
    str = obj->get_string("IV-Speed");
    if (str){ iv_speed  = IVCheckerFilter_string_to_enum(*str); }

    str = obj->get_string("Gender");
    if (str != nullptr){
        auto iter = GenderFilter_MAP.find(*str);
        if (iter != GenderFilter_MAP.end()){
            gender = iter->second;
        }
    }
}
JsonValue EggHatchFilterRow::to_json() const{
    JsonObject obj;
    obj["Action"] = EggHatchAction_NAMES[(size_t)action];
    obj["Shininess"] = EggHatchShinyFilter_NAMES[(size_t)shiny];
    obj["IV-HP"] = IVCheckerFilter_enum_to_string(iv_hp);
    obj["IV-Atk"] = IVCheckerFilter_enum_to_string(iv_atk);
    obj["IV-Def"] = IVCheckerFilter_enum_to_string(iv_def);
    obj["IV-SpAtk"] = IVCheckerFilter_enum_to_string(iv_spatk);
    obj["IV-SpDef"] = IVCheckerFilter_enum_to_string(iv_spdef);
    obj["IV-Speed"] = IVCheckerFilter_enum_to_string(iv_speed);
    obj["Gender"] = EggHatchGenderFilter_NAMES[(size_t)gender];

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
    box->addItem(QString::fromStdString(EggHatchAction_NAMES[0]));
    box->addItem(QString::fromStdString(EggHatchAction_NAMES[1]));
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
    box->addItem(QString::fromStdString(EggHatchShinyFilter_NAMES[0]));
    box->addItem(QString::fromStdString(EggHatchShinyFilter_NAMES[1]));
    box->addItem(QString::fromStdString(EggHatchShinyFilter_NAMES[2]));
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
    box->addItem(QString::fromStdString(EggHatchGenderFilter_NAMES[0]));
    box->addItem(QString::fromStdString(EggHatchGenderFilter_NAMES[1]));
    box->addItem(QString::fromStdString(EggHatchGenderFilter_NAMES[2]));
    box->setCurrentIndex((int)gender);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            if (index < 0){
                index = 0;
            }
            gender = (EggHatchGenderFilter)index;
        }
    );
    return box;
}


std::vector<std::string> EggHatchFilterOptionFactory::make_header() const{
    return std::vector<std::string>{
        "Action",
        "Shininess",
        "HP",
        "Attack",
        "Defense",
        "Sp. Attack",
        "Sp. Defense",
        "Speed",
        "Gender",
    };
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
        "IV checking requires that your right panel be set to the IV Judge and that you have selected the correct game language above.",
        m_factory, make_defaults()
    )
{}

void EggHatchFilterOption::load_json(const JsonValue& json){
    m_table.load_json(json);
}
JsonValue EggHatchFilterOption::to_json() const{
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

        if(filter.gender != gender && filter.gender != EggHatchGenderFilter::Any){
            continue;
        }

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
