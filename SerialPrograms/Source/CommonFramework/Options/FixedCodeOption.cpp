/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCodeOption.h"

namespace PokemonAutomation{



class FixedCodeWidget : public QWidget, public ConfigWidget{
public:
    FixedCodeWidget(QWidget& parent, FixedCodeOption& value);
    virtual void restore_defaults() override;

private:
    QString sanitized_code(const QString& text) const;

private:
    FixedCodeOption& m_value;
    QLineEdit* m_box;
};




FixedCodeOption::FixedCodeOption(
    QString label,
    size_t digits,
    QString default_value
)
    : m_label(std::move(label))
    , m_digits(digits)
    , m_default(default_value)
    , m_current(std::move(default_value))
{}

FixedCodeOption::operator const QString&() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
const QString& FixedCodeOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QString FixedCodeOption::set(QString x){
    QString error = check_validity(x);
    if (error.isEmpty()){
        m_current = std::move(x);
    }
    return error;
}

void FixedCodeOption::load_json(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
}
QJsonValue FixedCodeOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return QJsonValue(m_current);
}

void FixedCodeOption::to_str(uint8_t* code) const{
    SpinLockGuard lg(m_lock);
    QString qstr = sanitize_code(8, m_current);
    for (int c = 0; c < 8; c++){
        code[c] = qstr[c].unicode() - '0';
    }
}

QString FixedCodeOption::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
QString FixedCodeOption::check_validity(const QString& x) const{
    return validate_code(m_digits, x) ? QString() : "Code is invalid.";
}
void FixedCodeOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}

ConfigWidget* FixedCodeOption::make_ui(QWidget& parent){
    return new FixedCodeWidget(parent, *this);
}


QString FixedCodeWidget::sanitized_code(const QString& text) const{
    QString message;
    try{
        message = "Code: " + sanitize_code(m_value.m_digits, text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message_qt() + "</font>";
    }
    return message;
}
FixedCodeWidget::FixedCodeWidget(QWidget& parent, FixedCodeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    QString current = m_value.get();
    QLineEdit* box = new QLineEdit(current, this);
    right->addWidget(box);
    QLabel* under_text = new QLabel(sanitized_code(current), this);
    right->addWidget(under_text);
    under_text->setWordWrap(true);

    connect(
        box, &QLineEdit::textChanged,
        this, [=](const QString& text){
            m_value.set(text);
            under_text->setText(sanitized_code(text));
        }
    );
    connect(
        box, &QLineEdit::editingFinished,
        this, [=](){
            QString current = m_value.get();
            box->setText(current);
            under_text->setText(sanitized_code(current));
        }
    );
}
void FixedCodeWidget::restore_defaults(){
    m_value.restore_defaults();
    m_box->setText(m_value);
}



}
