/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "GroupOption.h"
#include "GroupWidget.h"

namespace PokemonAutomation{


GroupOption::GroupOption(
    QString label,
    bool toggleable,
    bool enabled
)
    : m_label(std::move(label))
    , m_toggleable(toggleable)
    , m_default_enabled(enabled)
    , m_enabled(enabled)
{}
inline ConfigWidget* GroupOption::make_ui(QWidget& parent){
    return new GroupWidget(parent, *this);
}
bool GroupOption::enabled() const{
    return m_enabled.load(std::memory_order_relaxed);
}
void GroupOption::load_json(const JsonValue& json){
    BatchOption::load_json(json);
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    if (m_toggleable){
        bool enabled;
        if (obj->read_boolean(enabled, "Enabled")){
            m_enabled.store(enabled, std::memory_order_relaxed);
            on_set_enabled(enabled);
        }
    }
}
JsonValue GroupOption::to_json() const{
    JsonObject obj = std::move(*BatchOption::to_json().get_object());
    if (m_toggleable){
        obj["Enabled"] = m_enabled.load(std::memory_order_relaxed);
    }
    return obj;
}
void GroupOption::restore_defaults(){
    m_enabled.store(m_default_enabled, std::memory_order_relaxed);
    BatchOption::restore_defaults();
}
void GroupOption::on_set_enabled(bool enabled){}


GroupWidget::GroupWidget(QWidget& parent, GroupOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    m_group_box = new QGroupBox(value.m_label, this);
    m_group_box->setCheckable(value.m_toggleable);
    m_group_box->setChecked(value.enabled());
    layout->addWidget(m_group_box);

#if 0
    QVBoxLayout* mid_layout = new QVBoxLayout(group_box);
    QWidget* mid_widget = new QWidget(group_box);
    mid_layout->addWidget(mid_widget);
    mid_layout-setContentsMargins(0, 0, 0, 0);

    QFont font = group_box->font();
    mid_widget->setFont(font);
    font.setBold(true);
    group_box->setFont(font);
#endif

    QVBoxLayout* group_layout = new QVBoxLayout(m_group_box);
    group_layout->setAlignment(Qt::AlignTop);
    group_layout->setContentsMargins(0, 10, 0, 0);

    m_expand_text = new QWidget(m_group_box);
    m_expand_text->setLayout(new QVBoxLayout());
    m_expand_text->layout()->addWidget(new QLabel("(double click to expand)", this));
    m_expand_text->setVisible(false);
    group_layout->addWidget(m_expand_text);


    m_options_holder = new QWidget(m_group_box);
    group_layout->addWidget(m_options_holder);
    m_options_layout = new QVBoxLayout(m_options_holder);
    m_options_layout->setContentsMargins(0, 0, 0, 0);

    for (auto& item : m_value.m_options){
        m_options.emplace_back(item.first->make_ui(parent));
        m_options.back()->widget().setContentsMargins(5, 5, 5, 5);
        m_options_layout->addWidget(&m_options.back()->widget());
    }

    connect(
        m_group_box, &QGroupBox::toggled,
        this, [=](bool on){
            m_value.m_enabled.store(on, std::memory_order_relaxed);
            m_value.on_set_enabled(on);
            on_set_enabled(on);
        }
    );
}
void GroupWidget::set_options_enabled(bool enabled){
    for (ConfigWidget* item : m_options){
        item->widget().setEnabled(enabled);
    }
}
void GroupWidget::restore_defaults(){
    bool on = m_value.m_default_enabled;
    m_value.m_enabled.store(on, std::memory_order_relaxed);
    m_group_box->setChecked(on);
    m_value.on_set_enabled(on);
    on_set_enabled(on);
    for (ConfigWidget* item : m_options){
        item->restore_defaults();
    }
}
void GroupWidget::update_ui(){
    bool on = m_value.m_enabled.load(std::memory_order_acquire);
    m_group_box->setChecked(on);
    for (ConfigWidget* item : m_options){
        item->update_ui();
    }
}
void GroupWidget::update_visibility(){
    ConfigWidget::update_visibility();
    for (ConfigWidget* item : m_options){
        item->update_visibility();
    }
}
void GroupWidget::on_set_enabled(bool enabled){}
void GroupWidget::mouseDoubleClickEvent(QMouseEvent* event){
    m_expand_text->setVisible(m_expanded);
    m_expanded = !m_expanded;
    m_options_holder->setVisible(m_expanded);
}




}
