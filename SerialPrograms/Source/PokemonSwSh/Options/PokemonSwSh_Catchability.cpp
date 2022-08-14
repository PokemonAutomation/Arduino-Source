/*  Catchability Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_Catchability.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;




class CatchabilitySelectorWidget : public QWidget, public ConfigWidget{
public:
    CatchabilitySelectorWidget(QWidget& parent, CatchabilitySelectorOption& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;

private:
    CatchabilitySelectorOption& m_value;
    QComboBox* m_box;
};




CatchabilitySelectorOption::CatchabilitySelectorOption()
    : m_label("<b>" + STRING_POKEMON + " Catchability</b>")
    , m_default(Catchability::ALWAYS_CATCHABLE)
    , m_current(Catchability::ALWAYS_CATCHABLE)
{}
void CatchabilitySelectorOption::load_json(const JsonValue& json){
    int index;
    if (json.read_integer(index, 0, 2)){
        m_current = (Catchability)index;
    }
}
JsonValue CatchabilitySelectorOption::to_json() const{
    return (int)m_current;
}

void CatchabilitySelectorOption::restore_defaults(){
    m_current = m_default;
}

ConfigWidget* CatchabilitySelectorOption::make_ui(QWidget& parent){
    return new CatchabilitySelectorWidget(parent, *this);
}




CatchabilitySelectorWidget::CatchabilitySelectorWidget(QWidget& parent, CatchabilitySelectorOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.m_label), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);
    m_box->addItem("Always Catchable");
    m_box->addItem("Maybe Uncatchable");
    m_box->addItem("Never Catchable");
    m_box->setCurrentIndex((int)m_value.m_current);
    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            m_value.m_current = (Catchability)index;
        }
    );
}
void CatchabilitySelectorWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void CatchabilitySelectorWidget::update_ui(){
    m_box->setCurrentIndex((int)(Catchability)m_value);
}



}
}
}
