/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
//#include "Common/Compiler.h"
#include "GroupWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ConfigWidget* GroupOption::make_QtWidget(QWidget& parent){
    return new GroupWidget(parent, *this);
}


GroupWidget::~GroupWidget(){
    m_value.remove_listener(*this);
}
GroupWidget::GroupWidget(QWidget& parent, GroupOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_restore_defaults_button(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    m_group_box = new QGroupBox(QString::fromStdString(value.label()), this);
    m_group_box->setCheckable(value.toggleable());
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

    for (auto& item : value.options()){
        m_options.emplace_back(item->make_QtWidget(parent));
        m_options.back()->widget().setContentsMargins(5, 5, 5, 5);
        m_options_layout->addWidget(&m_options.back()->widget());
    }

    if (value.restore_defaults_button_enabled()){
        m_restore_defaults_button = new QPushButton("Restore Defaults", this);
        m_restore_defaults_button->setContentsMargins(5, 5, 5, 5);
        QHBoxLayout* row = new QHBoxLayout();
        m_options_layout->addLayout(row);
        row->addWidget(m_restore_defaults_button, 1);
        row->addStretch(3);
        connect(
            m_restore_defaults_button, &QPushButton::clicked,
            this, [this](bool on){
                QMessageBox::StandardButton button = QMessageBox::question(
                    nullptr,
                    "Restore Defaults",
                    "Are you sure you wish to restore this section back to defaults?",
                    QMessageBox::Ok | QMessageBox::Cancel
                );
                if (button == QMessageBox::Ok){
                    m_value.restore_defaults();
                }
            }
        );
    }

    connect(
        m_group_box, &QGroupBox::toggled,
        this, [this](bool on){
            m_value.set_enabled(on);
//            m_value.on_set_enabled(on);
        }
    );

    value.add_listener(*this);
}

#if 0
void GroupWidget::set_options_enabled(bool enabled){
    for (ConfigWidget* item : m_options){
        item->widget().setEnabled(enabled);
    }
}
#endif
void GroupWidget::update_value(){
    bool on = m_value.enabled();
//    cout << "on = " << on << endl;
    m_group_box->setChecked(on);
    for (ConfigWidget* item : m_options){
        item->update_value();
    }
}
void GroupWidget::on_config_value_changed(void* object){
//    cout << "GroupWidget::value_changed()" << endl;
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
void GroupWidget::mouseDoubleClickEvent(QMouseEvent*){
    m_expand_text->setVisible(m_expanded);
    m_expanded = !m_expanded;
    m_options_holder->setVisible(m_expanded);
}




}
