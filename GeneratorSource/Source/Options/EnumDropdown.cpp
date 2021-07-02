/*  EnumDropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <QJsonArray>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "EnumDropdown.h"

namespace PokemonAutomation{


const QString EnumDropdown::OPTION_TYPE = "EnumDropdown";
const QString EnumDropdown::JSON_OPTIONS = "03-Options";


int EnumDropdown_init = register_option(
    EnumDropdown::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new EnumDropdown(obj)
        );
    }
);


EnumDropdown::EnumDropdown(const QJsonObject& obj)
    : SingleStatementOption(obj)
{
    QJsonArray options = json_get_array_throw(obj, JSON_OPTIONS);
    for (const auto option : options){
        if (!option.isArray()){
            PA_THROW_ParseException("Config Error - Expected Array: " + JSON_OPTIONS);
        }
        QJsonArray pair = option.toArray();
        if (pair.size() != 2){
            PA_THROW_ParseException("Config Error - Enum pairs should be 2 elements: " + JSON_OPTIONS);
        }
        if (!pair[0].isString() || !pair[1].isString()){
            PA_THROW_ParseException("Config Error - Enum pairs should be strings: " + JSON_OPTIONS);
        }
        m_options.emplace_back(
            pair[0].toString(),
            pair[1].toString()
        );
    }
    for (size_t c = 0; c < m_options.size(); c++){
        if (!m_map.emplace(m_options[c].first, c).second){
            PA_THROW_ParseException("Config Error - Duplicate option token.");
        }
    }
    {
        auto iter = m_map.find(json_get_string_throw(obj, JSON_DEFAULT));
        if (iter == m_map.end()){
            PA_THROW_ParseException("Config Error - Unrecognized token.");
        }
        m_default = iter->second;
    }
    {
        auto iter = m_map.find(json_get_string_throw(obj, JSON_CURRENT));
        if (iter == m_map.end()){
            PA_THROW_ParseException("Config Error - Unrecognized token.");
        }
        m_current = iter->second;
    }
}

void EnumDropdown::restore_defaults(){
    m_current = m_default;
}
QJsonObject EnumDropdown::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    {
        QJsonArray options;
        for (const auto& option : m_options){
            QJsonArray pair;
            pair += option.first;
            pair += option.second;
            options += std::move(pair);
        }
        root.insert(JSON_OPTIONS, std::move(options));
    }
    root.insert(JSON_DEFAULT, m_options[m_default].first);
    root.insert(JSON_CURRENT, m_options[m_current].first);
    return root;
}
std::string EnumDropdown::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = ";
    str += m_options[m_current].first.toUtf8().data();
    str += ";\r\n";
    return str;
}
QWidget* EnumDropdown::make_ui(QWidget& parent){
    return new EnumDropdownUI(parent, *this, m_label);
}


EnumDropdownUI::EnumDropdownUI(QWidget& parent, EnumDropdown& value, const QString& label)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(label, this);
    layout->addWidget(text);
    text->setWordWrap(true);
    QComboBox* box = new QComboBox(this);
    layout->addWidget(box);
    for (const auto& item : m_value.m_options){
        box->addItem(item.second);
    }
    box->setCurrentIndex(m_value.m_current);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){ m_value.m_current = index; }
    );
}
EnumDropdownUI::~EnumDropdownUI(){

}




}





