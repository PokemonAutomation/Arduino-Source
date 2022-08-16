/*  EnumDropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Tools/Tools.h"
#include "EnumDropdown.h"

namespace PokemonAutomation{
namespace HexGenerator{


const std::string EnumDropdown::OPTION_TYPE = "EnumDropdown";
const std::string EnumDropdown::JSON_OPTIONS = "03-Options";


int EnumDropdown_init = register_option(
    EnumDropdown::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new EnumDropdown(obj)
        );
    }
);


EnumDropdown::EnumDropdown(const JsonObject& obj)
    : SingleStatementOption(obj)
{
    const JsonArray& options = obj.get_array_throw(JSON_OPTIONS);
    for (const auto& option : options){
        const JsonArray& pair = option.get_array_throw();
        if (pair.size() != 2){
            throw ParseException("Config Error - Enum pairs should be 2 elements: " + JSON_OPTIONS);
        }
        m_options.emplace_back(
            pair[0].get_string_throw(),
            pair[1].get_string_throw()
        );
    }
    for (size_t c = 0; c < m_options.size(); c++){
        if (!m_map.emplace(m_options[c].first, c).second){
            throw ParseException("Config Error - Duplicate option token.");
        }
    }
    {
        auto iter = m_map.find(obj.get_string_throw(JSON_DEFAULT));
        if (iter == m_map.end()){
            throw ParseException("Config Error - Unrecognized token.");
        }
        m_default = iter->second;
    }
    {
        auto iter = m_map.find(obj.get_string_throw(JSON_CURRENT));
        if (iter == m_map.end()){
            throw ParseException("Config Error - Unrecognized token.");
        }
        m_current = iter->second;
    }
}

void EnumDropdown::restore_defaults(){
    m_current = m_default;
}
JsonObject EnumDropdown::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    {
        JsonArray options;
        for (const auto& option : m_options){
            JsonArray pair;
            pair.push_back(option.first);
            pair.push_back(option.second);
            options.push_back(std::move(pair));
        }
        root[JSON_OPTIONS] = std::move(options);
    }
    root[JSON_DEFAULT] = m_options[m_default].first;
    root[JSON_CURRENT] = m_options[m_current].first;
    return root;
}
std::string EnumDropdown::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = ";
    str += m_options[m_current].first;
    str += ";\r\n";
    return str;
}
QWidget* EnumDropdown::make_ui(QWidget& parent){
    return new EnumDropdownUI(parent, *this, m_label);
}


EnumDropdownUI::EnumDropdownUI(QWidget& parent, EnumDropdown& value, const std::string& label)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(QString::fromStdString(label), this);
    layout->addWidget(text);
    text->setWordWrap(true);
    QComboBox* box = new QComboBox(this);
    layout->addWidget(box);
    for (const auto& item : m_value.m_options){
        box->addItem(QString::fromStdString(item.second));
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
}





