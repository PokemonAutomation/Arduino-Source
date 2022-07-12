/*  RandomCode
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLineEdit>
#include <QIntValidator>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/CodeValidator.h"
#include "RandomCodeOption.h"

namespace PokemonAutomation{


const std::string RandomCode::OPTION_TYPE               = "RandomCode";
const std::string RandomCode::JSON_DECLARATION_RANDOM   = "02-DeclarationRandom";
const std::string RandomCode::JSON_DECLARATION_CODE     = "03-DeclarationCode";
const std::string RandomCode::JSON_DIGITS               = "04-Digits";
const std::string RandomCode::JSON_DEFAULT_RANDOM       = "05-DefaultRandom";
const std::string RandomCode::JSON_DEFAULT_CODE         = "06-DefaultCode";
const std::string RandomCode::JSON_CURRENT_RANDOM       = "07-CurrentRandom";
const std::string RandomCode::JSON_CURRENT_CODE         = "08-CurrentCode";

int RandomCode_init = register_option(
    RandomCode::OPTION_TYPE,
        [](const JsonObject& obj){
        return std::unique_ptr<ConfigItem>(
            new RandomCode(obj)
        );
    }
);


RandomCode::RandomCode(const JsonObject& obj)
    : ConfigItem(obj)
    , m_declaration_random(obj.get_string_throw(JSON_DECLARATION_RANDOM))
    , m_declaration_code(obj.get_string_throw(JSON_DECLARATION_CODE))
    , m_digits(obj.get_integer_throw(JSON_DIGITS))
    , m_default_random(obj.get_integer_throw(JSON_DEFAULT_RANDOM))
    , m_default_code(obj.get_string_throw(JSON_DEFAULT_CODE))
    , m_current_random(obj.get_integer_throw(JSON_CURRENT_RANDOM))
    , m_current_code(obj.get_string_throw(JSON_CURRENT_CODE))
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
JsonObject RandomCode::to_json() const{
    JsonObject root = ConfigItem::to_json();
    root[JSON_DECLARATION_RANDOM] = m_declaration_random;
    root[JSON_DECLARATION_CODE] = m_declaration_code;
    root[JSON_DIGITS] = m_digits;
    root[JSON_DEFAULT_RANDOM] = m_default_random;
    root[JSON_DEFAULT_CODE] = m_default_code;
    root[JSON_CURRENT_RANDOM] = m_current_random;
    root[JSON_CURRENT_CODE] = m_current_code;
    return root;
}
std::string RandomCode::to_cpp() const{
    std::string str;

    str += m_declaration_random;
    str += " = " + std::to_string(m_current_random);
    str += ";\r\n";

    str += m_declaration_code;
    str += " = \"";
    str += m_current_code;
    str += "\";\r\n";

    return str;
}
QWidget* RandomCode::make_ui(QWidget& parent){
    return new RandomCodeUI(parent, *this, m_label);
}


std::string RandomCodeUI::sanitized_code(const std::string& text) const{
    if (text.empty()){
        return "<font color=\"blue\">No Raid Code</font>";
    }
    std::string message;
    try{
        message = "Fixed Raid Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message() + "</font>";
    }
    return message;
}
std::string RandomCodeUI::random_code_string() const{
    std::string str;
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
        m_under_text->setText(QString::fromStdString(sanitized_code(m_value.m_current_code)));
    }else{
        m_label_code->setText("RNG Seed: ");
        m_under_text->setText(QString::fromStdString("Random Code: " + random_code_string()));
    }
}

RandomCodeUI::RandomCodeUI(QWidget& parent, RandomCode& value, const std::string& label)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(QString::fromStdString(label), this);
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

        box_code = new QLineEdit(QString::fromStdString(m_value.m_current_code), this);
        box_row->addWidget(box_code, 1);
    }
    m_under_text = new QLabel(QString::fromStdString(sanitized_code(m_value.m_current_code)), this);
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
            m_value.m_current_code = text.toStdString();
            update_labels();
        }
    );
}
RandomCodeUI::~RandomCodeUI(){

}



}
