/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QStyle>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "TextEditWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ConfigWidget* TextEditOption::make_QtWidget(QWidget& parent){
    return new TextEditWidget(parent, *this);
}



class TextEditWidget::Box : public QTextEdit{
public:
    Box(TextEditWidget& parent)
        : QTextEdit(&parent)
        , m_parent(parent)
    {
        this->setAcceptRichText(false);
        this->setFocusPolicy(Qt::StrongFocus);
        this->setPlaceholderText(QString::fromStdString(parent.m_value.placeholder_text()));

//        QPalette palette = this->palette();
//        palette.setColor(QPalette::Text, "blue");
//        palette.setColor(QPalette::PlaceholderText, "red");
//        this->setPalette(palette);

//        QColor default_color = this->textColor();

        if (parent.m_value.signal_all_text_changes()){
            connect(
                this, &QTextEdit::textChanged,
                [this]{
                    const std::string old_value = (std::string)m_parent.m_value;
                    std::string text = this->toPlainText().toStdString();
                    if (old_value == text){
                        return;
                    }
//                    cout << new_value << " : " << text << endl;
                    m_parent.m_value.set(std::move(text));
                }
            );
        }

//        this->hide();
    }

    virtual void focusInEvent(QFocusEvent* event) override{
        QTextEdit::focusInEvent(event);
        m_parent.m_value.report_focus_in();
    }
    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
//        static size_t c = 0;
//        cout << "focusOutEvent: " << c++ << endl;
        m_parent.m_value.set(this->toPlainText().toStdString());
    }

private:
    TextEditWidget& m_parent;
};





TextEditWidget::~TextEditWidget(){
    m_value.ConfigOption::remove_listener(*this);
}
TextEditWidget::TextEditWidget(QWidget& parent, TextEditOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* label = new QLabel(QString::fromStdString(value.label()), this);
    label->setWordWrap(true);
    layout->addWidget(label);
    m_box = new Box(*this);
    m_box->setText(QString::fromStdString(value));
    layout->addWidget(m_box);

    m_value.ConfigOption::add_listener(*this);
}
void TextEditWidget::update_value(){
    std::string new_value = (std::string)m_value;
    std::string text = m_box->toPlainText().toStdString();
    if (new_value == text){
        return;
    }
    m_box->setText(QString::fromStdString(m_value));
}
void TextEditWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        update_value();
    }, Qt::QueuedConnection);
}









}
