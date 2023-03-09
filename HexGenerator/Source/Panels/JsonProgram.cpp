/*  Program from JSON File.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Tools/Tools.h"
#include "JsonProgram.h"

namespace PokemonAutomation{
namespace HexGenerator{


Program_JsonFile::Program_JsonFile(std::string category, const std::string& filepath)
    : Program_JsonFile(std::move(category), load_json_file(filepath).get_object_throw())
{}
Program_JsonFile::Program_JsonFile(std::string category, const JsonObject& obj)
    : Program(std::move(category), obj)
{
    for (const auto& item : obj.get_array_throw(JSON_PARAMETERS)){
        m_options.emplace_back(parse_option(item.get_object_throw()));
    }
}

std::string Program_JsonFile::check_validity() const{
    for (const auto& item : m_options){
        std::string error = item->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void Program_JsonFile::restore_defaults(){
    for (const auto& item : m_options){
        item->restore_defaults();
    }
}

JsonArray Program_JsonFile::parameters_json() const{
    JsonArray params;
    for (const auto& item : m_options){
        params.push_back(item->to_json());
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
        QWidget* widget = item->make_ui(*box);
//        m_widgets.emplace_back(widget);
        layout->addWidget(widget);
    }
    return scroll;
}


}
}
