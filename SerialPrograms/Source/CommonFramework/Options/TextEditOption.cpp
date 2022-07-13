/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStyle>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "TextEditOption.h"

namespace PokemonAutomation{




class TextEditWidget : public QWidget, public ConfigWidget{
public:
    TextEditWidget(QWidget& parent, TextEditOption& value);

    virtual void update_ui() override;
    virtual void restore_defaults() override;

private:
    class Box;

    TextEditOption& m_value;
    QTextEdit* m_box;
};



TextEditOption::TextEditOption(
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_current(m_default)
{}

TextEditOption::operator const std::string&() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void TextEditOption::set(std::string x){
    SpinLockGuard lg(m_lock);
    m_current = std::move(x);
}


void TextEditOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = *str;
}
JsonValue TextEditOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void TextEditOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}
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

    void update_backing(){
        m_parent.m_value.set(this->toPlainText().toStdString());
    }

    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
        update_backing();
    }

private:
    TextEditWidget& m_parent;
};







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
}
void TextEditWidget::update_ui(){
    m_box->setText(QString::fromStdString(m_value));
}
void TextEditWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}









}
