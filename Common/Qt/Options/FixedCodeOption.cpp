/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCodeOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



class FixedCodeWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~FixedCodeWidget();
    FixedCodeWidget(QWidget& parent, FixedCodeOption& value);
    virtual void restore_defaults() override;
    virtual void update_ui() override;
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_box, [=]{
            update_ui();
        });
    }

private:
    std::string sanitized_code(const std::string& text) const;

private:
    FixedCodeOption& m_value;
    QLineEdit* m_box;
};




FixedCodeOption::FixedCodeOption(
    std::string label,
    size_t digits,
    std::string default_value
)
    : m_label(std::move(label))
    , m_digits(digits)
    , m_default(default_value)
    , m_current(std::move(default_value))
{}

FixedCodeOption::operator const std::string&() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
const std::string& FixedCodeOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
std::string FixedCodeOption::set(std::string x){
    std::string error = check_validity(x);
    if (!error.empty()){
        return error;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = std::move(x);
    }
    push_update();
    return std::string();
}

void FixedCodeOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = *str;
    }
    push_update();
}
JsonValue FixedCodeOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}

void FixedCodeOption::to_str(uint8_t* code) const{
    SpinLockGuard lg(m_lock);
    std::string qstr = sanitize_code(8, m_current);
    for (int c = 0; c < 8; c++){
        code[c] = qstr[c] - '0';
    }
}

std::string FixedCodeOption::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
std::string FixedCodeOption::check_validity(const std::string& x) const{
    return validate_code(m_digits, x) ? std::string() : "Code is invalid.";
}
void FixedCodeOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    push_update();
}

ConfigWidget* FixedCodeOption::make_ui(QWidget& parent){
    return new FixedCodeWidget(parent, *this);
}


std::string FixedCodeWidget::sanitized_code(const std::string& text) const{
    std::string message;
    try{
        message = "Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message() + "</font>";
    }
    return message;
}
FixedCodeWidget::~FixedCodeWidget(){
    m_value.remove_listener(*this);
}
FixedCodeWidget::FixedCodeWidget(QWidget& parent, FixedCodeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.m_label), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    std::string current = m_value.get();
    m_box = new QLineEdit(QString::fromStdString(current), this);
    right->addWidget(m_box);
    QLabel* under_text = new QLabel(QString::fromStdString(sanitized_code(current)), this);
    under_text->setWordWrap(true);
    right->addWidget(under_text);

    connect(
        m_box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            std::string str = text.toStdString();
            under_text->setText(QString::fromStdString(sanitized_code(str)));
//            m_value.set(str);
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        m_box, [=](){
            std::string current = m_box->text().toStdString();
            under_text->setText(QString::fromStdString(sanitized_code(current)));
//            m_box->setText(QString::fromStdString(current));
            m_value.set(current);
        }
    );
    m_value.add_listener(*this);
}
void FixedCodeWidget::restore_defaults(){
    m_value.restore_defaults();
}
void FixedCodeWidget::update_ui(){
    m_box->setText(QString::fromStdString(m_value));
}



}
