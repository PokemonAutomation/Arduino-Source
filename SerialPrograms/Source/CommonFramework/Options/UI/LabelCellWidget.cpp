/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include "LabelCellWidget.h"

namespace PokemonAutomation{



ConfigWidget* LabelCellOption::make_ui(QWidget& parent){
    return new LabelCellWidget(parent, *this);
}


LabelCellWidget::~LabelCellWidget(){
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
        QPixmap pixmap = QPixmap::fromImage(value.icon().to_QImage_ref());
//        QPixmap pixmap = QPixmap::fromImage(value.icon().scaled_to_QImage(25, 25));
        m_icon->setPixmap(pixmap);
        m_icon->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_icon);
    }

    m_text = new QLabel(QString::fromStdString(value.text()), this);
    layout->addWidget(m_text);
//    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
//    m_text->setOpenExternalLinks(true);
}




}
