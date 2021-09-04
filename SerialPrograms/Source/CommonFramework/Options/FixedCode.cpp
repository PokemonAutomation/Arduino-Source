/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCode.h"

namespace PokemonAutomation{


FixedCode::FixedCode(
    QString label,
    size_t digits,
    QString default_value
)
    : m_label(std::move(label))
    , m_digits(digits)
    , m_default(default_value)
    , m_current(std::move(default_value))
{}
void FixedCode::load_json(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    m_current = json.toString();
}
QJsonValue FixedCode::to_json() const{
    return QJsonValue(m_current);
}

void FixedCode::to_str(uint8_t* code) const{
    QString qstr = sanitize_code(8, m_current);
    for (int c = 0; c < 8; c++){
        code[c] = qstr[c].unicode() - '0';
    }
}

bool FixedCode::is_valid() const{
    return validate_code(m_digits, m_current);
}
void FixedCode::restore_defaults(){
    m_current = m_default;
}

ConfigOptionUI* FixedCode::make_ui(QWidget& parent){
    return new FixedCodeUI(parent, *this);
}


QString FixedCodeUI::sanitized_code(const QString& text) const{
    QString message;
    try{
        message = "Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message_qt() + "</font>";
    }
    return message;
}
FixedCodeUI::FixedCodeUI(QWidget& parent, FixedCode& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
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
void FixedCodeUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(m_value);
}



}
