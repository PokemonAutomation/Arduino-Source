/*  Text Edit (hidden for sensitive data)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include "HiddenTextEditOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


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
ConfigOptionUI* HiddenTextEditOption::make_ui(QWidget& parent){
    return new HiddenTextEditOptionUI(parent, *this);
}






class HiddenTextEditOptionUI::Box : public QTextEdit{
public:
    Box(HiddenTextEditOptionUI& parent)
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
    HiddenTextEditOptionUI& m_parent;
};







HiddenTextEditOptionUI::HiddenTextEditOptionUI(QWidget& parent, HiddenTextEditOption& value)
    : QWidget(&parent)
    , ConfigOptionUI(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
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
void HiddenTextEditOptionUI::restore_defaults(){
    m_value.restore_defaults();
}









}
