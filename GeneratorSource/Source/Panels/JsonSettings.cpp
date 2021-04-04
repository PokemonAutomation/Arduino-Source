/*  Settings from JSON File.
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
#include "JsonSettings.h"

namespace PokemonAutomation{


Settings_JsonFile::Settings_JsonFile(const QString& filepath)
    : Settings_JsonFile(read_json_file(filepath).object())
{}
Settings_JsonFile::Settings_JsonFile(const QJsonObject& obj)
    : ConfigSet(obj)
{
    for (const auto item : json_get_array_throw(obj, JSON_OPTIONS)){
        if (!item.isObject()){
            throw StringException("Config Error - Expected and object.");
        }
        m_options.emplace_back(parse_option(item.toObject()));
    }
}

bool Settings_JsonFile::is_valid() const{
    for (const auto& item : m_options){
        if (!item->is_valid()){
            return false;
        }
    }
    return true;
}
void Settings_JsonFile::restore_defaults(){
    for (const auto& item : m_options){
        item->restore_defaults();
    }
}

QJsonArray Settings_JsonFile::options_json() const{
    QJsonArray params;
    for (const auto& item : m_options){
        params += item->to_json();
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
        layout->addWidget(item->make_ui(*box));
    }
    return scroll;
}


}


