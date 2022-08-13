/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "Common/Cpp/Json/JsonValue.h"
#include "BooleanCheckBoxOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



BooleanCheckBoxOption::BooleanCheckBoxOption(
    std::string label,
    bool default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
{}


void BooleanCheckBoxOption::operator=(bool x){
    m_current.store(x, std::memory_order_relaxed);
    push_update();
}
void BooleanCheckBoxOption::load_json(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        *this = value;
        push_update();
    }
}
JsonValue BooleanCheckBoxOption::to_json() const{
    return (bool)*this;
}

void BooleanCheckBoxOption::restore_defaults(){
    *this = m_default;
    push_update();
}




class BooleanCheckBoxWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~BooleanCheckBoxWidget(){
        m_value.remove_listener(*this);
    }
    BooleanCheckBoxWidget(QWidget& parent, BooleanCheckBoxOption& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
        , m_value(value)
        , m_box(new QCheckBox(this))
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
        text->setWordWrap(true);
        layout->addWidget(text, 3);
        m_box = new QCheckBox(this);
        m_box->setChecked(m_value);
        layout->addWidget(m_box, 1);
        connect(
            m_box, &QCheckBox::stateChanged,
            this, [=](int){
                m_value = m_box->isChecked();
            }
        );
        value.add_listener(*this);
    }
    virtual void restore_defaults() override{
        m_value.restore_defaults();
    }
    virtual void update_ui() override{
        if (m_value != m_box->isChecked()){
            m_box->setChecked(m_value);
        }
    }
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_box, [=]{
            update_ui();
        });
    }

private:
    BooleanCheckBoxOption& m_value;
    QCheckBox* m_box;
};



ConfigWidget* BooleanCheckBoxOption::make_ui(QWidget& parent){
    return new BooleanCheckBoxWidget(parent, *this);
}


}
