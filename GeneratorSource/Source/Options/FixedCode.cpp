/*  FixedCode
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCode.h"

namespace PokemonAutomation{


const QString FixedCode::OPTION_TYPE = "FixedCode";
const QString FixedCode::JSON_DIGITS = "03-Digits";

int FixedCode_init = register_option(
    FixedCode::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new FixedCode(obj)
        );
    }
);


FixedCode::FixedCode(const QJsonObject& obj)
    : SingleStatementOption(obj)
    , m_digits(json_get_int(obj, JSON_DIGITS))
    , m_default(json_get_string(obj, JSON_DEFAULT))
    , m_current(json_get_string(obj, JSON_CURRENT))
{
    if (!validate_code(m_digits, m_default)){
        throw StringException("Invalid code.");
    }
    if (!validate_code(m_digits, m_current)){
        throw StringException("Invalid code.");
    }
}
void FixedCode::restore_defaults(){
    m_current = m_default;
}
bool FixedCode::is_valid() const{
    return validate_code(m_digits, m_current);
}
QJsonObject FixedCode::to_json() const{
    QJsonObject root = SingleStatementOption::to_json();
    root.insert(JSON_DIGITS, (int)m_digits);
    root.insert(JSON_DEFAULT, m_default);
    root.insert(JSON_CURRENT, m_current);
    return root;
}
std::string FixedCode::to_cpp() const{
    std::string str;
    str += m_declaration.toUtf8().data();
    str += " = \"";
    str += m_current.toUtf8().data();
    str += "\";\r\n";
    return str;
}
QWidget* FixedCode::make_ui(QWidget& parent){
    return new FixedCodeUI(parent, *this, m_label);
}



QString FixedCodeUI::sanitized_code(const QString& text){
    QString message;
    try{
        message = "Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const StringException& str){
        message = "<font color=\"red\">" + str.message() + "</font>";
    }
    return message;
}

FixedCodeUI::FixedCodeUI(QWidget& parent, FixedCode& value, const QString& label)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    QLineEdit* box = new QLineEdit(m_value.m_current, this);
    right->addWidget(box);
    QLabel* under_text = new QLabel(sanitized_code(m_value.m_current), this);
    right->addWidget(under_text);
    under_text->setWordWrap(true);

    connect(
        box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            m_value.m_current = text;
            under_text->setText(sanitized_code(text));
        }
    );
}
FixedCodeUI::~FixedCodeUI(){

}



}
