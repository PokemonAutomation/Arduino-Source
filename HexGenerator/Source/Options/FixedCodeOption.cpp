/*  FixedCode
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCodeOption.h"

namespace PokemonAutomation{


const std::string FixedCode::OPTION_TYPE = "FixedCode";
const std::string FixedCode::JSON_DIGITS = "03-Digits";

int FixedCode_init = register_option(
    FixedCode::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new FixedCode(obj)
        );
    }
);


FixedCode::FixedCode(const JsonObject& obj)
    : SingleStatementOption(obj)
    , m_digits(obj.get_integer_throw(JSON_DIGITS))
    , m_default(obj.get_string_throw(JSON_DEFAULT))
    , m_current(obj.get_string_throw(JSON_CURRENT))
{
    if (!validate_code(m_digits, m_default)){
        throw ParseException("Invalid code.");
    }
    if (!validate_code(m_digits, m_current)){
        throw ParseException("Invalid code.");
    }
}
void FixedCode::restore_defaults(){
    m_current = m_default;
}
QString FixedCode::check_validity() const{
    return validate_code(m_digits, m_current) ? QString() : "Code is invalid.";
}
JsonObject FixedCode::to_json() const{
    JsonObject root = SingleStatementOption::to_json();
    root[JSON_DIGITS] = m_digits;
    root[JSON_DEFAULT] = m_default;
    root[JSON_CURRENT] = m_current;
    return root;
}
std::string FixedCode::to_cpp() const{
    std::string str;
    str += m_declaration;
    str += " = \"";
    str += m_current;
    str += "\";\r\n";
    return str;
}
QWidget* FixedCode::make_ui(QWidget& parent){
    return new FixedCodeUI(parent, *this, m_label);
}



std::string FixedCodeUI::sanitized_code(const std::string& text){
    std::string message;
    try{
        message = "Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message() + "</font>";
    }
    return message;
}

FixedCodeUI::FixedCodeUI(QWidget& parent, FixedCode& value, const std::string& label)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(QString::fromStdString(label), this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    QLineEdit* box = new QLineEdit(QString::fromStdString(m_value.m_current), this);
    right->addWidget(box);
    QLabel* under_text = new QLabel(QString::fromStdString(sanitized_code(m_value.m_current)), this);
    right->addWidget(under_text);
    under_text->setWordWrap(true);

    connect(
        box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            m_value.m_current = text.toStdString();
            under_text->setText(QString::fromStdString(sanitized_code(m_value.m_current)));
        }
    );
}
FixedCodeUI::~FixedCodeUI(){

}



}
