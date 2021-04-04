/*  Program from JSON File.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "JsonProgram.h"

namespace PokemonAutomation{


Program_JsonFile::Program_JsonFile(const QString& filepath)
    : Program_JsonFile(read_json_file(filepath).object())
{}
Program_JsonFile::Program_JsonFile(const QJsonObject& obj)
    : Program(obj)
{
    for (const auto item : json_get_array_throw(obj, JSON_PARAMETERS)){
        if (!item.isObject()){
            throw StringException("Config Error - Expected and object.");
        }
        m_options.emplace_back(parse_option(item.toObject()));
    }
}

bool Program_JsonFile::is_valid() const{
    for (const auto& item : m_options){
        if (!item->is_valid()){
            return false;
        }
    }
    return true;
}
void Program_JsonFile::restore_defaults(){
    for (const auto& item : m_options){
        item->restore_defaults();
    }
}

QJsonArray Program_JsonFile::parameters_json() const{
    QJsonArray params;
    for (const auto& item : m_options){
        params += item->to_json();
    }
    return params;
}
std::string Program_JsonFile::parameters_cpp() const{
    std::string str;
    for (const auto& item : m_options){
        str += item->to_cpp();
    }
    return str;
}

QWidget* Program_JsonFile::make_options_body(QWidget& parent){
    QScrollArea* scroll = new QScrollArea(&parent);
    scroll->setWidgetResizable(true);

    QWidget* box = new QWidget(scroll);
    box->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout* layout = new QVBoxLayout(box);
    box->setLayout(layout);

    if (m_options.empty()){
        QLabel* label = new QLabel("There are no program-specific options for this program.");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }else{
        layout->setAlignment(Qt::AlignTop);
    }

    scroll->setWidget(box);

    for (const auto& item : m_options){
        layout->addWidget(item->make_ui(*box));
    }
    return scroll;
}


}
