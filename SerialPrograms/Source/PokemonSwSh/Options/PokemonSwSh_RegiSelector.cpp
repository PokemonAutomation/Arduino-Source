/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "Common/Qt/NoWheelComboBox.h"
#include "PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class RegiSelectorWidget : public QWidget, public ConfigWidget{
public:
    RegiSelectorWidget(QWidget& parent, RegiSelectorOption& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;

private:
    RegiSelectorOption& m_value;
    QComboBox* m_box;
};



RegiSelectorOption::RegiSelectorOption()
    : m_label("<b>Name of Regi:</b>")
    , m_default(RegiGolem::Registeel)
    , m_current(RegiGolem::Registeel)
{}
void RegiSelectorOption::load_json(const QJsonValue& json){
    size_t index = json.toInt((int)m_default);
    index = std::min(index, (size_t)4);
    m_current = (RegiGolem)index;
}
QJsonValue RegiSelectorOption::to_json() const{
    return QJsonValue((int)m_current);
}

void RegiSelectorOption::restore_defaults(){
    m_current = m_default;
};

ConfigWidget* RegiSelectorOption::make_ui(QWidget& parent){
    return new RegiSelectorWidget(parent, *this);
}




RegiSelectorWidget::RegiSelectorWidget(QWidget& parent, RegiSelectorOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(m_value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);
    m_box->addItem("Regirock");
    m_box->addItem("Regice");
    m_box->addItem("Registeel");
    m_box->addItem("Regieleki");
    m_box->addItem("Regidrago");
    m_box->setCurrentIndex((int)m_value.m_current);
    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            m_value.m_current = (RegiGolem)index;
        }
    );
}
void RegiSelectorWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void RegiSelectorWidget::update_ui(){
    m_box->setCurrentIndex((int)(RegiGolem)m_value);
}



}
}
}

