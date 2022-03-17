/*  RandomCode
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QIntValidator>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/CodeValidator.h"
#include "RandomCodeOption.h"

namespace PokemonAutomation{


const QString RandomCode::OPTION_TYPE               = "RandomCode";
const QString RandomCode::JSON_DECLARATION_RANDOM   = "02-DeclarationRandom";
const QString RandomCode::JSON_DECLARATION_CODE     = "03-DeclarationCode";
const QString RandomCode::JSON_DIGITS               = "04-Digits";
const QString RandomCode::JSON_DEFAULT_RANDOM       = "05-DefaultRandom";
const QString RandomCode::JSON_DEFAULT_CODE         = "06-DefaultCode";
const QString RandomCode::JSON_CURRENT_RANDOM       = "07-CurrentRandom";
const QString RandomCode::JSON_CURRENT_CODE         = "08-CurrentCode";

int RandomCode_init = register_option(
    RandomCode::OPTION_TYPE,
        [](const QJsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new RandomCode(obj)
        );
    }
);


RandomCode::RandomCode(const QJsonObject& obj)
    : ConfigItem(obj)
    , m_declaration_random(json_get_string_throw(obj, JSON_DECLARATION_RANDOM))
    , m_declaration_code(json_get_string_throw(obj, JSON_DECLARATION_CODE))
    , m_digits(json_get_int_throw(obj, JSON_DIGITS))
    , m_default_random(json_get_int_throw(obj, JSON_DEFAULT_RANDOM))
    , m_default_code(json_get_string_throw(obj, JSON_DEFAULT_CODE))
    , m_current_random(json_get_int_throw(obj, JSON_CURRENT_RANDOM))
    , m_current_code(json_get_string_throw(obj, JSON_CURRENT_CODE))
{
    if (m_default_random > m_digits){
        m_default_random = m_digits;
    }
    if (m_current_random > m_digits){
        m_current_random = m_digits;
    }
    if (!validate_code(m_digits, m_default_code)){
        throw ParseException("Invalid code.");
    }
    if (!validate_code(m_digits, m_current_code)){
        throw ParseException("Invalid code.");
    }
}
void RandomCode::restore_defaults(){
    m_current_random = m_default_random;
    m_current_code = m_default_code;
}
QString RandomCode::check_validity() const{
    if (m_current_random == 0){
        return validate_code(m_digits, m_current_code)
            ? QString()
            : "Code is invalid.";
    }else{
        return m_current_random <= m_digits
            ? QString()
            : "Random digits cannot be greater than " + QString::number(m_digits) + ".";
    }
}
QJsonObject RandomCode::to_json() const{
    QJsonObject root = ConfigItem::to_json();
    root.insert(JSON_DECLARATION_RANDOM, m_declaration_random);
    root.insert(JSON_DECLARATION_CODE, m_declaration_code);
    root.insert(JSON_DIGITS, (int)m_digits);
    root.insert(JSON_DEFAULT_RANDOM, (int)m_default_random);
    root.insert(JSON_DEFAULT_CODE, m_default_code);
    root.insert(JSON_CURRENT_RANDOM, (int)m_current_random);
    root.insert(JSON_CURRENT_CODE, m_current_code);
    return root;
}
std::string RandomCode::to_cpp() const{
    std::string str;

    str += m_declaration_random.toUtf8().data();
    str += " = " + std::to_string(m_current_random);
    str += ";\r\n";

    str += m_declaration_code.toUtf8().data();
    str += " = \"";
    str += m_current_code.toUtf8().data();
    str += "\";\r\n";

    return str;
}
QWidget* RandomCode::make_ui(QWidget& parent){
    return new RandomCodeUI(parent, *this, m_label);
}


QString RandomCodeUI::sanitized_code(const QString& text) const{
    if (text.isEmpty()){
        return "<font color=\"blue\">No Raid Code</font>";
    }
    QString message;
    try{
        message = "Fixed Raid Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + QString::fromStdString(e.message()) + "</font>";
    }
    return message;
}
QString RandomCodeUI::random_code_string() const{
    QString str;
    char ch = 'A' - 1;
    size_t c = 0;
    for (; c < m_value.m_current_random; c++){
        ch++;
        str += ch;
    }
    for (; c < m_value.m_digits; c++){
        str += ch;
    }
    return str;
}
void RandomCodeUI::update_labels(){
    if (m_value.m_current_random == 0){
        m_label_code->setText("Raid Code: ");
        m_under_text->setText(sanitized_code(m_value.m_current_code));
    }else{
        m_label_code->setText("RNG Seed: ");
        m_under_text->setText("Random Code: " + random_code_string());
    }
}

RandomCodeUI::RandomCodeUI(QWidget& parent, RandomCode& value, const QString& label)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    QLineEdit* box_random;
    {
        QHBoxLayout* box_row = new QHBoxLayout();
        right->addLayout(box_row);

        box_row->addWidget(new QLabel("Random Digits: ", this), 1);

        box_random = new QLineEdit(QString::number(m_value.m_current_random), this);
        box_row->addWidget(box_random, 1);
        box_random->setValidator(new QIntValidator(0, m_value.m_digits, this));
    }
    QLineEdit* box_code;
    {
        QHBoxLayout* box_row = new QHBoxLayout();
        right->addLayout(box_row);

        m_label_code = new QLabel("Raid Code: ", this);
        box_row->addWidget(m_label_code, 1);

        box_code = new QLineEdit(m_value.m_current_code, this);
        box_row->addWidget(box_code, 1);
    }
    m_under_text = new QLabel(sanitized_code(m_value.m_current_code), this);
    right->addWidget(m_under_text);
    update_labels();

    connect(
        box_random, &QLineEdit::textChanged,
        this, [=](const QString& text){
            int read = text.toInt();
            if (read < 0){
                read = 0;
                box_random->setText(QString::number(read));
            }
            if (read > (int)m_value.m_digits){
                read = (int)m_value.m_digits;
                box_random->setText(QString::number(read));
            }
            m_value.m_current_random = read;
            update_labels();
        }
    );
    connect(
        box_code, &QLineEdit::textChanged,
        this, [=](const QString& text){
            m_value.m_current_code = text;
            update_labels();
        }
    );
}
RandomCodeUI::~RandomCodeUI(){

}



}
