/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include "LabelCellWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


template class RegisterConfigWidget<LabelCellWidget>;


LabelCellWidget::~LabelCellWidget(){
    m_value.remove_listener(*this);
}
LabelCellWidget::LabelCellWidget(QWidget& parent, LabelCellOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
//    layout->setAlignment(Qt::AlignLeft);

    if (value.icon()){
        m_icon = new QLabel(this);
        Resolution resolution = value.resolution();
//        cout << resolution.width << " x " << resolution.height << endl;
        QPixmap pixmap;
        if (resolution.width == 0 || resolution.height == 0){
            pixmap = QPixmap::fromImage(value.icon().to_QImage_ref());
        }else{
            pixmap = QPixmap::fromImage(value.icon().scaled_to_QImage(resolution.width, resolution.height));
        }
        m_icon->setPixmap(pixmap);
        m_icon->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_icon);
    }

    m_text = new QLabel(QString::fromStdString(value.text()), this);
    layout->addWidget(m_text, 1);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
//    m_text->setOpenExternalLinks(true);

    m_value.add_listener(*this);
}

void LabelCellWidget::update_value(){
    m_text->setText(QString::fromStdString(m_value.text()));
}
void LabelCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_text, [this]{
        update_value();
    }, Qt::QueuedConnection);
}




}
