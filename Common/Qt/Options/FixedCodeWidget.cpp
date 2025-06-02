/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCodeWidget.h"

namespace PokemonAutomation{


ConfigWidget* FixedCodeOption::make_QtWidget(QWidget& parent){
    return new FixedCodeWidget(parent, *this);
}




std::string FixedCodeWidget::sanitized_code(const std::string& text) const{
    std::string message;
    try{
        message = "Code: " + sanitize_code(m_value.digits(), text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message() + "</font>";
    }
    return message;
}
FixedCodeWidget::~FixedCodeWidget(){
    m_value.remove_listener(*this);
}
FixedCodeWidget::FixedCodeWidget(QWidget& parent, FixedCodeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    std::string current = m_value.get();
    m_box = new QLineEdit(QString::fromStdString(current), this);
    right->addWidget(m_box);
    QLabel* under_text = new QLabel(QString::fromStdString(sanitized_code(current)), this);
    under_text->setWordWrap(true);
    right->addWidget(under_text);

    connect(
        m_box, &QLineEdit::textChanged,
        this, [this, under_text](const QString& text){
            std::string str = text.toStdString();
            under_text->setText(QString::fromStdString(sanitized_code(str)));
//            m_value.set(str);
        }
    );
    connect(
        m_box, &QLineEdit::editingFinished,
        m_box, [this, under_text](){
            std::string current = m_box->text().toStdString();
            under_text->setText(QString::fromStdString(sanitized_code(current)));
//            m_box->setText(QString::fromStdString(current));
            m_value.set(current);
        }
    );
    m_value.add_listener(*this);
}
void FixedCodeWidget::update_value(){
    m_box->setText(QString::fromStdString(m_value));
}
void FixedCodeWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





}
