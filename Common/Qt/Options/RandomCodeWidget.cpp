/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QIntValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/CodeValidator.h"
#include "RandomCodeWidget.h"

namespace PokemonAutomation{



ConfigWidget* RandomCodeOption::make_ui(QWidget& parent){
    return new RandomCodeWidget(parent, *this);
}


RandomCodeWidget::~RandomCodeWidget(){
    m_value.remove_listener(*this);
}
RandomCodeWidget::RandomCodeWidget(QWidget& parent, RandomCodeOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.m_label), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    QVBoxLayout* right = new QVBoxLayout();
    layout->addLayout(right, 1);

    {
        QHBoxLayout* box_row = new QHBoxLayout();
        right->addLayout(box_row);

        box_row->addWidget(new QLabel("Random Digits: ", this), 1);

        m_box_random = new QLineEdit(QString::number(m_value.m_current.random_digits()), this);
        box_row->addWidget(m_box_random, 1);
        m_box_random->setValidator(new QIntValidator(0, (int)m_value.m_current.total_digits(), this));
    }
    {
        QHBoxLayout* box_row = new QHBoxLayout();
        right->addLayout(box_row);

        m_label_code = new QLabel("Raid Code: ", this);
        box_row->addWidget(m_label_code, 1);

        m_box_code = new QLineEdit(QString::fromStdString(m_value.m_current.code_string()), this);
        box_row->addWidget(m_box_code, 1);
    }
    m_under_text = new QLabel(QString::fromStdString(sanitized_code(m_value.m_current.code_string())), this);
    right->addWidget(m_under_text);
    update_labels();

    connect(
        m_box_random, &QLineEdit::textChanged,
        this, [=](const QString& text){
            int read = text.toInt();
            RaidCodeOption code = m_value;
            code.m_random_digits = read;
            m_value.set(code);
        }
    );
    connect(
        m_box_code, &QLineEdit::textChanged,
        this, [=](const QString& text){
            RaidCodeOption code = m_value;
            code.m_code = text.toStdString();
            m_value.set(code);
        }
    );

    m_value.add_listener(*this);
}
std::string RandomCodeWidget::sanitized_code(const std::string& text) const{
    if (text.empty()){
        return "<font color=\"blue\">No Raid Code</font>";
    }
    std::string message;
    try{
        message = "Fixed Raid Code: " + sanitize_code(m_value.m_current.total_digits(), text);
    }catch (const ParseException& e){
        message = "<font color=\"red\">" + e.message() + "</font>";
    }
    return message;
}
std::string RandomCodeWidget::random_code_string() const{
    std::string str;
    char ch = 'A' - 1;
    size_t c = 0;
    for (; c < m_value.m_current.random_digits(); c++){
        ch++;
        str += ch;
    }
    for (; c < m_value.m_current.total_digits(); c++){
        str += ch;
    }
    return str;
}
void RandomCodeWidget::update_labels(){
    if (m_value.m_current.random_digits() == 0){
//        m_label_code->setText("Raid Code: ");
//        m_code_row->setEnabled(true);
        m_box_code->setEnabled(true);
        m_under_text->setText(QString::fromStdString(sanitized_code(m_value.m_current.code_string())));
    }else{
//        m_label_code->setText("RNG Seed: ");
//        m_code_row->setEnabled(false);
        m_box_code->setEnabled(false);
        m_under_text->setText(QString::fromStdString("Random Code: " + random_code_string()));
    }
}
void RandomCodeWidget::restore_defaults(){
    m_value.restore_defaults();
}
void RandomCodeWidget::update_ui(){
    m_box_random->setText(QString::number(m_value.m_current.random_digits()));
    m_box_code->setText(QString::fromStdString(m_value.m_current.code_string()));
    update_labels();
}
void RandomCodeWidget::value_changed(){
    QMetaObject::invokeMethod(this, [=]{
        update_ui();
    }, Qt::QueuedConnection);
}




}
