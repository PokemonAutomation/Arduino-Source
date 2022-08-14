/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStyle>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "TextEditWidget.h"

namespace PokemonAutomation{



ConfigWidget* TextEditOption::make_ui(QWidget& parent){
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

#if 0
        connect(
            this, &QTextEdit::textChanged,
            [=]{
                style()->polish(this);
            }
        );
#endif

//        this->hide();
    }

    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
        m_parent.m_value.set(this->toPlainText().toStdString());
    }

private:
    TextEditWidget& m_parent;
};





TextEditWidget::~TextEditWidget(){
    m_value.remove_listener(*this);
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

    m_value.add_listener(*this);
}
void TextEditWidget::update_ui(){
    m_box->setText(QString::fromStdString(m_value));
}
void TextEditWidget::restore_defaults(){
    m_value.restore_defaults();
}
void TextEditWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update_ui();
    });
}









}
