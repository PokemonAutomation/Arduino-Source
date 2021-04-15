/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <time.h>
#include <QJsonObject>
#include <QIntValidator>
#include <QHBoxLayout>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/CodeValidator.h"
#include "RandomCode.h"

namespace PokemonAutomation{


RaidCode::RaidCode()
    : m_digits(8)
    , m_random_digits(0)
    , m_code("1280 0000")
{}
bool RaidCode::get_code(uint8_t* code) const{
    if (m_random_digits == 0 && m_code.size() <= 0){
        return false;
    }
    if (m_random_digits == 0){
        QString qstr = sanitize_code(8, m_code);
        for (int c = 0; c < 8; c++){
            code[c] = qstr[c].unicode() - '0';
        }
        return true;
    }
    srand(time(nullptr));
    for (uint8_t c = 0; c < m_random_digits; c++){
        uint8_t x;
        do{
            x = rand() & 0x0f;
        }while (x >= 10);
        code[c] = x;
    }
    for (size_t c = m_random_digits; c < m_digits; c++){
        code[c] = code[c - 1];
    }
    return true;
}
bool RaidCode::is_valid() const{
    if (m_random_digits == 0){
        return validate_code(m_digits, m_code);
    }else{
        return m_random_digits <= m_digits;
    }
}


RandomCode::RandomCode()
    : ConfigOption(
        "<b>Raid Code:</b><br>Blank for no raid code. Set random digits to zero for a fixed code. Otherwise, it is the # of leading random digits."
    )
{}
void RandomCode::load_json(const QJsonValue& json){
    QJsonObject root = json.toObject();
    json_get_int(m_current.m_random_digits, root, "RandomDigits");
    json_get_string(m_current.m_code, root, "RaidCode");
}
QJsonValue RandomCode::to_json() const{
    QJsonObject root;
    root.insert("RandomDigits", (int)m_current.m_random_digits);
    root.insert("RaidCode", m_current.m_code);
    return root;
}

bool RandomCode::get_code(uint8_t* code) const{
    return m_current.get_code(code);
}

bool RandomCode::is_valid() const{
    return m_current.is_valid();
}
void RandomCode::restore_defaults(){
    m_current = m_default;
}

ConfigOptionUI* RandomCode::make_ui(QWidget& parent){
    return new RandomCodeUI(parent, *this);
}


RandomCodeUI::RandomCodeUI(QWidget& parent, RandomCode& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    {
        QHBoxLayout* box_row = new QHBoxLayout();
        right->addLayout(box_row);

        box_row->addWidget(new QLabel("Random Digits: ", this), 1);

        m_box_random = new QLineEdit(QString::number(m_value.m_current.random_digits()), this);
        box_row->addWidget(m_box_random, 1);
        m_box_random->setValidator(new QIntValidator(0, (int)m_value.m_current.total_digits(), this));
    }
    {
        QHBoxLayout* box_row = new QHBoxLayout();
        right->addLayout(box_row);

        m_label_code = new QLabel("Raid Code: ", this);
        box_row->addWidget(m_label_code, 1);

        m_box_code = new QLineEdit(m_value.m_current.code_string(), this);
        box_row->addWidget(m_box_code, 1);
    }
    m_under_text = new QLabel(sanitized_code(m_value.m_current.code_string()), this);
    right->addWidget(m_under_text);
    update_labels();

    connect(
        m_box_random, &QLineEdit::textChanged,
        this, [=](const QString& text){
            int read = text.toInt();
            if (read < 0){
                read = 0;
                m_box_random->setText(QString::number(read));
            }
            if (read > (int)m_value.m_current.total_digits()){
                read = (int)m_value.m_current.total_digits();
                m_box_random->setText(QString::number(read));
            }
            m_value.m_current.m_random_digits = read;
            update_labels();
        }
    );
    connect(
        m_box_code, &QLineEdit::textChanged,
        this, [=](const QString& text){
            m_value.m_current.m_code = text;
            update_labels();
        }
    );
}
QString RandomCodeUI::sanitized_code(const QString& text) const{
    if (text.isEmpty()){
        return "<font color=\"blue\">No Raid Code</font>";
    }
    QString message;
    try{
        message = "Fixed Raid Code: " + sanitize_code(m_value.m_current.total_digits(), text);
    }catch (const StringException& str){
        message = "<font color=\"red\">" + str.message() + "</font>";
    }
    return message;
}
QString RandomCodeUI::random_code_string() const{
    QString str;
    char ch = 'A' - 1;
    size_t c = 0;
    for (; c < m_value.m_current.random_digits(); c++){
        ch++;
        str += ch;
    }
    for (; c < m_value.m_current.total_digits(); c++){
        str += ch;
    }
    return str;
}
void RandomCodeUI::update_labels(){
    if (m_value.m_current.random_digits() == 0){
//        m_label_code->setText("Raid Code: ");
//        m_code_row->setEnabled(true);
        m_box_code->setEnabled(true);
        m_under_text->setText(sanitized_code(m_value.m_current.code_string()));
    }else{
//        m_label_code->setText("RNG Seed: ");
//        m_code_row->setEnabled(false);
        m_box_code->setEnabled(false);
        m_under_text->setText("Random Code: " + random_code_string());
    }
}
void RandomCodeUI::restore_defaults(){
    m_value.restore_defaults();
    m_box_random->setText(QString::number(m_value.m_current.random_digits()));
    m_box_code->setText(m_value.m_current.code_string());
}




}









