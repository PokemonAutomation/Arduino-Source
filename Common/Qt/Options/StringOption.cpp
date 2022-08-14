/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "StringOption.h"

namespace PokemonAutomation{



StringOption::StringOption(
    bool is_password,
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
    : m_label(std::move(label))
    , m_is_password(is_password)
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_current(m_default)
{}

StringOption::operator std::string() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void StringOption::set(std::string x){
    {
        SpinLockGuard lg(m_lock);
        m_current = std::move(x);
    }
    push_update();
}

void StringOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr) {
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = *str;
    }
    push_update();
}
JsonValue StringOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}

void StringOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    push_update();
}




class StringWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StringWidget(){
        m_value.remove_listener(*this);
    }
    StringWidget(QWidget& parent, StringOption& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
        , m_value(value)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
        text->setWordWrap(true);
        layout->addWidget(text, 1);

        m_box = new QLineEdit(QString::fromStdString(m_value));
        m_box->setPlaceholderText(QString::fromStdString(value.placeholder_text()));
        layout->addWidget(m_box, 1);

        if (m_value.is_password()){
            m_box->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        }

        connect(
            m_box, &QLineEdit::editingFinished,
            this, [=](){
                m_value.set(m_box->text().toStdString());
            }
        );

        m_value.add_listener(*this);
    }
    virtual void restore_defaults() override{
        m_value.restore_defaults();
    }
    virtual void update_ui() override{
        m_box->setText(QString::fromStdString(m_value));
    }
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_box, [=]{
            update_ui();
        });
    }

private:
    StringOption& m_value;
    QLineEdit * m_box;
};


ConfigWidget* StringOption::make_ui(QWidget& parent){
    return new StringWidget(parent, *this);
}




}
