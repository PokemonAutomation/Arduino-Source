/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <time.h>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/CodeValidator.h"
#include "RandomCodeOption.h"

namespace PokemonAutomation{



class RandomCodeWidget : public QWidget, public ConfigWidget{
public:
    RandomCodeWidget(QWidget& parent, RandomCodeOption& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;

private:
    QString sanitized_code(const QString& text) const;
    QString random_code_string() const;
    void update_labels();

private:
    RandomCodeOption& m_value;
    QLabel* m_label_code;
    QLabel* m_under_text;
    QLineEdit* m_box_random;
    QLineEdit* m_box_code;
};




RaidCodeOption::RaidCodeOption()
    : m_digits(8)
    , m_random_digits(0)
    , m_code("1280 0000")
{}
RaidCodeOption::RaidCodeOption(size_t random_digits, QString code_string)
    : m_digits(8)
    , m_random_digits(random_digits)
    , m_code(std::move(code_string))
{}
QString RaidCodeOption::check_validity() const{
    if (m_random_digits == 0){
        return validate_code(m_digits, m_code)
            ? QString()
            : "Code is invalid.";
    }else{
        return m_random_digits <= m_digits
            ? QString()
            : "Random digits cannot be greater than " + QString::number(m_digits) + ".";
    }
}
bool RaidCodeOption::code_enabled() const{
    return m_random_digits != 0 || m_code.size() > 0;
}
bool RaidCodeOption::get_code(uint8_t* code) const{
    if (!code_enabled()){
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


RandomCodeOption::RandomCodeOption()
    : m_label(
        "<b>Raid Code:</b><br>Blank for no raid code. Set random digits to zero for a fixed code. Otherwise, it is the # of leading random digits."
    )
{}
RandomCodeOption::RandomCodeOption(QString label, size_t random_digits, QString code_string)
    : m_label(std::move(label))
    , m_default(random_digits, std::move(code_string))
    , m_current(m_default)
{}
void RandomCodeOption::load_json(const JsonValue2& json){
    const JsonObject2* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_integer(m_current.m_random_digits, "RandomDigits");

    std::string code;
    if (obj->read_string(code, "RaidCode")){
        m_current.m_code = QString::fromStdString(code);
    }
}
JsonValue2 RandomCodeOption::to_json() const{
    JsonObject2 obj;
    obj["RandomDigits"] = m_current.m_random_digits;
    obj["RaidCode"] = m_current.m_code.toStdString();
    return obj;
}

bool RandomCodeOption::code_enabled() const{
    return m_current.code_enabled();
}

bool RandomCodeOption::get_code(uint8_t* code) const{
    return m_current.get_code(code);
}

QString RandomCodeOption::check_validity() const{
    return m_current.check_validity();
}
void RandomCodeOption::restore_defaults(){
    m_current = m_default;
}

ConfigWidget* RandomCodeOption::make_ui(QWidget& parent){
    return new RandomCodeWidget(parent, *this);
}


RandomCodeWidget::RandomCodeWidget(QWidget& parent, RandomCodeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(value.m_label, this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

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
QString RandomCodeWidget::sanitized_code(const QString& text) const{
    if (text.isEmpty()){
        return "<font color=\"blue\">No Raid Code</font>";
    }
    QString message;
    try{
        message = "Fixed Raid Code: " + sanitize_code(m_value.m_current.total_digits(), text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + QString::fromStdString(e.message()) + "</font>";
    }
    return message;
}
QString RandomCodeWidget::random_code_string() const{
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
void RandomCodeWidget::update_labels(){
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
void RandomCodeWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void RandomCodeWidget::update_ui(){
    m_box_random->setText(QString::number(m_value.m_current.random_digits()));
    m_box_code->setText(m_value.m_current.code_string());
}




}









