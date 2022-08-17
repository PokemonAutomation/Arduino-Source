/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCompleter>
#include "StringSelectWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



ConfigWidget* StringSelectCell::make_ui(QWidget& parent){
    return new StringSelectCellWidget(parent, *this);
}
ConfigWidget* StringSelectOption2::make_ui(QWidget& parent){
    return new StringSelectOptionWidget2(parent, *this);
}




StringSelectCellWidget::~StringSelectCellWidget(){
    m_value.remove_listener(*this);
}
StringSelectCellWidget::StringSelectCellWidget(QWidget& parent, StringSelectCell& value)
    : NoWheelComboBox(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    this->setEditable(true);
    this->setInsertPolicy(QComboBox::NoInsert);
    this->completer()->setCompletionMode(QCompleter::PopupCompletion);
    this->completer()->setFilterMode(Qt::MatchContains);

    const StringSelectEntry& entry = value.entry();
    this->addItem(entry.icon, QString::fromStdString(entry.display_name));
    this->setCurrentIndex(0);

    StringSelectCellWidget::update();

    connect(
        this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [=](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.set_by_name(this->itemText(index).toStdString());
//            cout << "index = " << index << endl;
        }
    );

    value.add_listener(*this);
}

void StringSelectCellWidget::load_options(){
//    cout << "load_options()" << endl;
    if (this->count() <= 1){
        this->clear();
        for (const auto& item : m_value.database().case_list()){
            this->addItem(item.icon, QString::fromStdString(item.display_name));
        }
    }
    this->setCurrentIndex((int)m_value.index());
}
void StringSelectCellWidget::hide_options(){
//    cout << "hide_options()" << endl;
    do{
        //  All options shown.
        if (this->count() > 1){
            break;
        }

        //  No valid index.
        int ui_index = this->currentIndex();
        if (ui_index < 0){
            break;
        }

        //  Mismatching selection.
        std::string name = this->itemText(ui_index).toStdString();
        if (name != m_value.display_name()){
            break;
        }

        return;
    }while (false);

    //  Remove all elements and add the one that is selected.
    this->clear();
    const StringSelectEntry& entry = m_value.entry();
    this->addItem(entry.icon, QString::fromStdString(entry.display_name));
    this->setCurrentIndex(0);
}
void StringSelectCellWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent()" << endl;
    update();
    NoWheelComboBox::focusInEvent(event);
}
void StringSelectCellWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent()" << endl;
    NoWheelComboBox::focusOutEvent(event);
//    update();
}
void StringSelectCellWidget::update(){
    ConfigWidget::update();
    if (hasFocus()){
        load_options();
    }else{
        hide_options();
    }
}
void StringSelectCellWidget::value_changed(){
    QMetaObject::invokeMethod(this, [=]{
        update();
    }, Qt::QueuedConnection);
}





StringSelectOptionWidget2::StringSelectOptionWidget2(QWidget& parent, StringSelectOption2& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_cell(new StringSelectCellWidget(parent, value))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    layout->addWidget(m_cell, 1);
}
void StringSelectOptionWidget2::update(){
    ConfigWidget::update();
    m_cell->update();
}





}
