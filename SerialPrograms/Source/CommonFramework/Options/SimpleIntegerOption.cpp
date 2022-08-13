/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "SimpleIntegerOption.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    std::string label,
    Type default_value,
    Type min_value,
    Type max_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
{}

template <typename Type>
std::string SimpleIntegerOption<Type>::set(Type x){
    std::string err = check_validity(x);
    if (err.empty()){
        m_current.store(x, std::memory_order_relaxed);
        push_update();
    }
    return err;
}
template <typename Type>
void SimpleIntegerOption<Type>::load_json(const JsonValue& json){
    Type value;
    if (json.read_integer(value, m_min_value, m_max_value)){
        set(value);
        m_current.store(value, std::memory_order_relaxed);
        push_update();
    }
}
template <typename Type>
JsonValue SimpleIntegerOption<Type>::to_json() const{
    return (Type)*this;
}

template <typename Type>
std::string SimpleIntegerOption<Type>::check_validity(Type x) const{
    if (x < m_min_value){
        return "Value too small: min = " + std::to_string(m_min_value) + ", value = " + std::to_string(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + std::to_string(m_max_value) + ", value = " + std::to_string(x);
    }
    return std::string();
}
template <typename Type>
std::string SimpleIntegerOption<Type>::check_validity() const{
    return check_validity(*this);
}
template <typename Type>
void SimpleIntegerOption<Type>::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    push_update();
}





template <typename Type>
class SimpleIntegerWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~SimpleIntegerWidget(){
        m_value.remove_listener(*this);
    }
    SimpleIntegerWidget(QWidget& parent, SimpleIntegerOption<Type>& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
        , m_value(value)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
        text->setWordWrap(true);
        layout->addWidget(text, 1);
        m_box = new QLineEdit(QString::number(m_value), this);
    //    box->setInputMask("999999999");
    //    QIntValidator* validator = new QIntValidator(value.m_min_value, value.m_max_value, box);
    //    box->setValidator(validator);
        layout->addWidget(m_box, 1);
        connect(
            m_box, &QLineEdit::textChanged,
            this, [=](const QString& text){
                bool ok;
                Type current = (Type)text.toLong(&ok);
                QPalette palette;
                if (ok && m_value.check_validity(current).empty()){
                    palette.setColor(QPalette::Text, Qt::black);
                }else{
                    palette.setColor(QPalette::Text, Qt::red);
                }
                m_box->setPalette(palette);
            }
        );
        connect(
            m_box, &QLineEdit::editingFinished,
            this, [=](){
                bool ok;
                Type current = (Type)m_box->text().toLong(&ok);
                QPalette palette;
                if (ok && m_value.check_validity(current).empty()){
                    palette.setColor(QPalette::Text, Qt::black);
                }else{
                    palette.setColor(QPalette::Text, Qt::red);
                }
                m_box->setPalette(palette);

                if (current == m_value){
                    return;
                }

                m_value.set(current);
            }
        );
        value.add_listener(*this);
    }
    virtual void restore_defaults() override{
        m_value.restore_defaults();
    }
    virtual void update_ui() override{
        m_box->setText(QString::number(m_value));
    }
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_box, [=]{
            update_ui();
        });
    }

private:
    SimpleIntegerOption<Type>& m_value;
    QLineEdit* m_box;
};


template <typename Type>
ConfigWidget* SimpleIntegerOption<Type>::make_ui(QWidget& parent){
    return new SimpleIntegerWidget<Type>(parent, *this);
}




template class SimpleIntegerOption<uint8_t>;
template class SimpleIntegerOption<uint16_t>;
template class SimpleIntegerOption<uint32_t>;
template class SimpleIntegerOption<uint64_t>;
template class SimpleIntegerOption<int8_t>;

template class SimpleIntegerWidget<uint8_t>;
template class SimpleIntegerWidget<uint16_t>;
template class SimpleIntegerWidget<uint32_t>;
template class SimpleIntegerWidget<uint64_t>;
template class SimpleIntegerWidget<int8_t>;


}
