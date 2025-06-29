/*  MAC Address Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QLineEdit>
#include "ConfigWidget.h"
#include "MacAddressWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


MacAddressCellWidget::~MacAddressCellWidget(){
    m_value.remove_listener(*this);
}

MacAddressCellWidget::MacAddressCellWidget(QWidget& parent, MacAddressCell& value)
    : QLineEdit(QString::fromStdString(value.to_string()), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
//    cout << "sizeHint() = " << this->sizeHint().width() << endl;

    this->setReadOnly(value.lock_mode() == LockMode::READ_ONLY);

    connect(
        this, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(this->text().toStdString());
        }
    );
    value.add_listener(*this);
}
void MacAddressCellWidget::update_value(){
    this->setText(QString::fromStdString(m_value.to_string()));
}
void MacAddressCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}



}
