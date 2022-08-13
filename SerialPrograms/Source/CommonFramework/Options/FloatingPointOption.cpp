/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "FloatingPointOption.h"

namespace PokemonAutomation{



FloatingPointOption::FloatingPointOption(
    std::string label,
    double default_value,
    double min_value,
    double max_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}

std::string FloatingPointOption::set(double x){
    std::string err = check_validity(x);
    if (err.empty()){
        m_current.store(x, std::memory_order_relaxed);
        push_update();
    }
    return err;
}

void FloatingPointOption::load_json(const JsonValue& json){
    double value;
    if (!json.read_float(value)){
        return;
    }
    value = std::max(value, m_min_value);
    value = std::min(value, m_max_value);
    if (std::isnan(value)){
        value = m_default;
    }
    m_current.store(value, std::memory_order_relaxed);
    push_update();
}
JsonValue FloatingPointOption::to_json() const{
    return (double)*this;
}

std::string FloatingPointOption::check_validity(double x) const{
    if (x < m_min_value){
        std::ostringstream ss;
        return "Value too small: min = " + tostr_default(m_min_value) + ", value = " + tostr_default(x);
    }
    if (x > m_max_value){
        std::ostringstream ss;
        return "Value too large: max = " + tostr_default(m_max_value) + ", value = " + tostr_default(x);
    }
    if (std::isnan(x)){
        return "Value is NaN.";
    }
    return std::string();
}
std::string FloatingPointOption::check_validity() const{
    return check_validity(*this);
}
void FloatingPointOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    push_update();
}




class FloatingPointWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~FloatingPointWidget(){
        m_value.remove_listener(*this);
    }
    FloatingPointWidget(QWidget& parent, FloatingPointOption& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
        , m_value(value)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
        text->setWordWrap(true);
        layout->addWidget(text, 1);
        m_box = new QLineEdit(QString::number(m_value, 'f'), this);
    //    box->setInputMask("999999999");
    //    QDoubleValidator* validator = new QDoubleValidator(value.min_value(), value.max_value(), 2, this);
    //    m_box->setValidator(validator);
        layout->addWidget(m_box, 1);
        connect(
            m_box, &QLineEdit::textChanged,
            this, [=](const QString& text){
                bool ok;
                double current = text.toDouble(&ok);
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
                double current = m_box->text().toDouble(&ok);
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
        m_box->setText(QString::number(m_value, 'f'));
    }
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_box, [=]{
            update_ui();
        });
    }

private:
    FloatingPointOption& m_value;
    QLineEdit* m_box;
};


ConfigWidget* FloatingPointOption::make_ui(QWidget& parent){
    return new FloatingPointWidget(parent, *this);
}



}
