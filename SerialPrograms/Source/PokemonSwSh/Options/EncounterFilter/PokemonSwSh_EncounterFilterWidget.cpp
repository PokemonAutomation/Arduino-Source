/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "PokemonSwSh_EncounterFilterEnums.h"
#include "PokemonSwSh_EncounterFilterWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



#if 0
EncounterFilterWidget::EncounterFilterWidget(QWidget& parent, EncounterFilterOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
//    QLabel* text = new QLabel(value.label(), this);
//    layout->addWidget(text);

    {
//        QWidget* widget = new QWidget(this);

        QHBoxLayout* hbox = new QHBoxLayout();
        layout->addLayout(hbox);
        hbox->addWidget(new QLabel("<b>Stop on:</b>"));

        m_shininess = new NoWheelComboBox(this);
        hbox->addWidget(m_shininess);
        if (m_value.m_rare_stars){
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::ANYTHING]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::ANY_SHINY]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::STAR_ONLY]));
        }else{
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::ANYTHING]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::NOT_SHINY]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::ANY_SHINY]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::STAR_ONLY]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::SQUARE_ONLY]));
            m_shininess->addItem(QString::fromStdString(ShinyFilter_NAMES[(int)ShinyFilter::NOTHING]));
        }
        ShinyFilter current = m_value.m_shiny_filter_current;
        for (int c = 0; c < m_shininess->count(); c++){
            if (m_shininess->itemText(c).toStdString() == ShinyFilter_NAMES[(int)current]){
                m_shininess->setCurrentIndex(c);
                break;
            }
        }
        connect(
            m_shininess, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
                if (index < 0){
                    return;
                }

                std::string text = m_shininess->itemText(index).toStdString();
                auto iter = ShinyFilter_MAP.find(text);
                if (iter == ShinyFilter_MAP.end()){
                    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid option: " + text);
                }
                m_value.m_shiny_filter_current = iter->second;
            }
        );
    }

    if (m_value.m_enable_overrides){
        layout->addSpacing(5);
        m_table = value.m_table.make_QtWidget(*this);
        layout->addWidget(&m_table->widget());
    }
}
void EncounterFilterWidget::update_value(){
    ShinyFilter current = m_value.m_shiny_filter_current;
    for (int c = 0; c < m_shininess->count(); c++){
        if (m_shininess->itemText(c).toStdString() == ShinyFilter_NAMES[(int)current]){
            m_shininess->setCurrentIndex(c);
            break;
        }
    }
}
void EncounterFilterWidget::update_visibility(){
    ConfigWidget::update_visibility();
    if (m_table){
        m_table->update_visibility();
    }
}
#endif



}
}
}
