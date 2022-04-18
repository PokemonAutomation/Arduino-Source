/*  Text Edit (hidden for sensitive data)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QTextEdit>
#include "HiddenTextEditOption.h"

namespace PokemonAutomation{



class HiddenTextEditWidget : public QWidget, public ConfigWidget{
public:
    HiddenTextEditWidget(QWidget& parent, HiddenTextEditOption& value);
    virtual void restore_defaults() override;

private:
    class Box;

    HiddenTextEditOption& m_value;
    QTextEdit* m_box;
};




HiddenTextEditOption::HiddenTextEditOption(
    QString label,
    QString default_value,
    QString placeholder_text
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_current(m_default)
{}

HiddenTextEditOption::operator const QString&() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QString HiddenTextEditOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void HiddenTextEditOption::set(QString x){
    SpinLockGuard lg(m_lock);
    m_current = std::move(x);
}

void HiddenTextEditOption::load_json(const QJsonValue& json){
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
}
QJsonValue HiddenTextEditOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void HiddenTextEditOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}
ConfigWidget* HiddenTextEditOption::make_ui(QWidget& parent){
    return new HiddenTextEditWidget(parent, *this);
}






class HiddenTextEditWidget::Box : public QTextEdit{
public:
    Box(HiddenTextEditWidget& parent)
        : QTextEdit(&parent)
        , m_parent(parent)
    {
        this->setAcceptRichText(false);
        this->setFocusPolicy(Qt::StrongFocus);
        this->setPlaceholderText(parent.m_value.placeholder_text());
//        this->hide();

        connect(
            this, &QTextEdit::textChanged,
            this, &Box::resize
        );
    }

    void update_backing(){
        m_parent.m_value.set(this->toPlainText());
    }

    void resize(){
        QSize size = this->document()->size().toSize();
        this->setFixedHeight(
            std::max(size.height() + 3, 40)
        );
    }

    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
        update_backing();
    }

private:
    HiddenTextEditWidget& m_parent;
};







HiddenTextEditWidget::HiddenTextEditWidget(QWidget& parent, HiddenTextEditOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(new QLabel(value.label(), this));

    QCheckBox* checkbox = new QCheckBox("Show Text (Do not show this to the public.)", this);
    checkbox->setChecked(value.get().isEmpty());
    layout->addWidget(checkbox);

    Box* box = new Box(*this);
    m_box = box;
    box->setText(value);
    box->setVisible(checkbox->isChecked());
    box->resize();
    layout->addWidget(box);

    connect(
        checkbox, &QCheckBox::stateChanged,
        this, [=](int){
            m_box->setVisible(checkbox->isChecked());
            box->resize();
        }
    );
}
void HiddenTextEditWidget::restore_defaults(){
    m_value.restore_defaults();
}









}
