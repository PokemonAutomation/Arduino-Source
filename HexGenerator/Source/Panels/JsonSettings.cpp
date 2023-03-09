/*  Settings from JSON File.
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
#include "JsonSettings.h"

namespace PokemonAutomation{
namespace HexGenerator{


Settings_JsonFile::~Settings_JsonFile(){
    for (QWidget* widget : m_widgets){
        delete widget;
    }
}
Settings_JsonFile::Settings_JsonFile(std::string category, const std::string& filepath)
    : Settings_JsonFile(std::move(category), load_json_file(filepath).get_object_throw(filepath))
{}
Settings_JsonFile::Settings_JsonFile(std::string category, const JsonObject& obj)
    : ConfigSet(std::move(category), obj)
{
    for (const auto& item : obj.get_array_throw(JSON_OPTIONS)){
        const JsonObject& obj = item.get_object_throw();
        m_options.emplace_back(parse_option(obj));
    }
}

std::string Settings_JsonFile::check_validity() const{
    for (const auto& item : m_options){
        std::string error = item->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void Settings_JsonFile::restore_defaults(){
    for (const auto& item : m_options){
        item->restore_defaults();
    }
}

JsonArray Settings_JsonFile::options_json() const{
    JsonArray params;
    for (const auto& item : m_options){
        params.push_back(item->to_json());
    }
    return params;
}
std::string Settings_JsonFile::options_cpp() const{
    std::string str;
    for (const auto& item : m_options){
        str += item->to_cpp();
    }
    return str;
}

QWidget* Settings_JsonFile::make_options_body(QWidget& parent){
    QScrollArea* scroll = new QScrollArea(&parent);
    scroll->setWidgetResizable(true);

    QWidget* box = new QWidget(scroll);
    box->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout* layout = new QVBoxLayout(box);
    layout->setAlignment(Qt::AlignTop);
    box->setLayout(layout);

    scroll->setWidget(box);

    for (const auto& item : m_options){
        QWidget* widget = item->make_ui(*box);
        m_widgets.emplace_back(widget);
        layout->addWidget(widget);
    }
    return scroll;
}



}
}


