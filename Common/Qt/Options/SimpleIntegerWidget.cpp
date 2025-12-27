/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "ConfigWidget.h"
#include "SimpleIntegerWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




SimpleIntegerCellWidget::~SimpleIntegerCellWidget(){
    m_value.remove_listener(*this);
}
SimpleIntegerCellWidget::SimpleIntegerCellWidget(QWidget& parent, SimpleIntegerCellBase& value)
    : QLineEdit(QString::number(value.current_value()), &parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
//    cout << "sizeHint() = " << this->sizeHint().width() << endl;

    this->setReadOnly(value.lock_mode() == LockMode::READ_ONLY);

    connect(
        this, &QLineEdit::textChanged,
        this, [this](const QString& text){
            bool ok;
            NativeType current = (NativeType)text.toLong(&ok);
            QPalette palette;
            if (ok && m_value.check_validity(current).empty()){
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            this->setPalette(palette);
        }
    );
    connect(
        this, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            if (std::is_unsigned_v<NativeType>){
                uint64_t current = this->text().toULongLong(&ok);
                current = std::max<uint64_t>(current, m_value.min_value());
                current = std::min<uint64_t>(current, m_value.max_value());
                this->setText(QString::number(current));
                m_value.set((NativeType)current);
            }else{
                int64_t current = this->text().toLongLong(&ok);
                current = std::max<int64_t>(current, m_value.min_value());
                current = std::min<int64_t>(current, m_value.max_value());
                this->setText(QString::number(current));
                m_value.set((NativeType)current);
            }
        }
    );
    value.add_listener(*this);
}
void SimpleIntegerCellWidget::update_value(){
    this->setText(QString::number(m_value.current_value()));
}
void SimpleIntegerCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}







SimpleIntegerOptionWidget::~SimpleIntegerOptionWidget(){
    m_value.remove_listener(*this);
}
SimpleIntegerOptionWidget::SimpleIntegerOptionWidget(QWidget& parent, SimpleIntegerOptionBase& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_cell(new SimpleIntegerCellWidget(*this, value))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);
    layout->addWidget(m_cell, 1);
    value.add_listener(*this);
}
void SimpleIntegerOptionWidget::update_value(){
    m_cell->update_value();
}
void SimpleIntegerOptionWidget::on_config_value_changed(void* object){
    m_cell->on_config_value_changed(object);
}






}
