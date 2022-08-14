/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include "Common/Cpp/Json/JsonValue.h"
#include "StaticTextOption.h"

namespace PokemonAutomation{




StaticTextOption::StaticTextOption(std::string label)
    : m_text(std::move(label))
{}
std::string StaticTextOption::text() const{
    SpinLockGuard lg(m_lock);
    return m_text;
}
void StaticTextOption::set_text(std::string label){
    {
        SpinLockGuard lg(m_lock);
        m_text = std::move(label);
    }
    push_update();
}
void StaticTextOption::load_json(const JsonValue&){
}
JsonValue StaticTextOption::to_json() const{
    return JsonValue();
}



SectionDividerOption::SectionDividerOption(std::string label)
    : m_text(std::move(label))
{}
std::string SectionDividerOption::text() const{
    SpinLockGuard lg(m_lock);
    return m_text;
}
void SectionDividerOption::set_text(std::string label){
    {
        SpinLockGuard lg(m_lock);
        m_text = std::move(label);
    }
    push_update();
}
void SectionDividerOption::load_json(const JsonValue&){
}
JsonValue SectionDividerOption::to_json() const{
    return JsonValue();
}





class StaticTextWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StaticTextWidget(){
        m_value.remove_listener(*this);
    }
    StaticTextWidget(QWidget& parent, StaticTextOption& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
        , m_value(value)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        m_text = new QLabel(QString::fromStdString(value.text()), this);
        m_text->setWordWrap(true);
        layout->addWidget(m_text);
    //    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
        m_text->setOpenExternalLinks(true);

        m_value.add_listener(*this);
    }
    virtual void update_ui() override{
        m_text->setText(QString::fromStdString(m_value.text()));
    }
    virtual void restore_defaults() override{}
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_text, [=]{
            update_ui();
        });
    }

private:
    StaticTextOption& m_value;
    QLabel* m_text;
};


class SectionDividerWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~SectionDividerWidget(){
        m_value.remove_listener(*this);
    }
    SectionDividerWidget(QWidget& parent, SectionDividerOption& value)
        : QWidget(&parent)
        , ConfigWidget(value, *this)
        , m_value(value)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 10, 0, 0);

        QFrame* frame = new QFrame(this);
        layout->addWidget(frame);
        frame->setFrameShape(QFrame::HLine);

        m_text = new QLabel(QString::fromStdString(value.text()), this);
        m_text->setWordWrap(true);
        layout->addWidget(m_text);
    //    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
        m_text->setOpenExternalLinks(true);

        m_value.add_listener(*this);
    }
    virtual void update_ui() override{
        m_text->setText(QString::fromStdString(m_value.text()));
    }
    virtual void restore_defaults() override{}
    virtual void value_changed() override{
        QMetaObject::invokeMethod(m_text, [=]{
            update_ui();
        });
    }

private:
    SectionDividerOption& m_value;
    QLabel* m_text;
};




ConfigWidget* StaticTextOption::make_ui(QWidget& parent){
    return new StaticTextWidget(parent, *this);
}
ConfigWidget* SectionDividerOption::make_ui(QWidget& parent){
    return new SectionDividerWidget(parent, *this);
}






}
