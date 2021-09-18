/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "PokemonSwSh_EggStepCount.h"

namespace PokemonAutomation{


const std::vector<uint16_t> STEP_COUNTS{
    1280,
    2560,
    3840,
    5120,
    6400,
    7680,
    8960,
    10240,
    20480,
    30720,
};

EggStepCountOption::EggStepCountOption()
    : m_label("<b>Step Count:</b><br>Lookup the # of steps on Serebii.")
    , m_default(3)
    , m_current(3)
{}
void EggStepCountOption::load_json(const QJsonValue& json){
    size_t index = json.toInt((int)m_default);
    index = std::min(index, STEP_COUNTS.size());
    m_current = index;
}
QJsonValue EggStepCountOption::to_json() const{
    return QJsonValue((int)m_current);
}


void EggStepCountOption::restore_defaults(){
    m_current = m_default;
}

ConfigOptionUI* EggStepCountOption::make_ui(QWidget& parent){
    return new EggStepCountUI(parent, *this);
}


EggStepCountUI::EggStepCountUI(QWidget& parent, EggStepCountOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* text = new QLabel(m_value.m_label, this);
    layout->addWidget(text, 1);
    text->setWordWrap(true);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);
    for (uint16_t count : STEP_COUNTS){
        m_box->addItem(QString::fromStdString(PokemonAutomation::tostr_u_commas(count)));
    }
    m_box->setCurrentIndex((int)m_value.m_current);
    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            m_value.m_current = index;
        }
    );
}
void EggStepCountUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->setCurrentIndex((int)m_value.m_current);
}



}

