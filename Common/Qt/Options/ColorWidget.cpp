/*  Color Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QLabel>
#include <QLineEdit>
#include "QHBoxLayout"
#include "ColorWidget.h"

namespace PokemonAutomation{


ConfigWidget* ColorCell::make_QtWidget(QWidget& parent){
    return new ColorCellWidget(parent, *this);
}


ColorCellWidget::~ColorCellWidget(){
    m_value.remove_listener(*this);
}
ColorCellWidget::ColorCellWidget(QWidget& parent, ColorCell& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 5, 0);

    QString str = QString::fromStdString(value.to_str());

    m_line_edit = new QLineEdit(str, this);
    m_line_edit->setFixedWidth(60);
    layout->addWidget(m_line_edit);

    m_box = new QLabel(this);
    m_box->setTextFormat(Qt::RichText);
    layout->addWidget(m_box);

    ColorCellWidget::update_value();

    connect(
        m_line_edit, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(m_line_edit->text().toStdString());
            update_value();
        }
    );

    m_value.add_listener(*this);
}
void ColorCellWidget::update_value(){
    QString str = QString::fromStdString(m_value.to_str());
    m_line_edit->setText(str);
    m_box->setText("<font color=\"#" + str + "\">&#x2b24;</font>");
}
void ColorCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}




}
