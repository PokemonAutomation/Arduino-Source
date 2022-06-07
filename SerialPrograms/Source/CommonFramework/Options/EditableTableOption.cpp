/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include "EditableTableOption.h"
#include "EditableTableWidget.h"

namespace PokemonAutomation{



EditableTableOption::EditableTableOption(
    QString label, const EditableTableFactory& factory,
    std::vector<std::unique_ptr<EditableTableRow>> default_value
)
    : EditableTableBaseOption(std::move(label), factory, std::move(default_value))
{}

void EditableTableOption::load_json(const QJsonValue& json){
    return EditableTableBaseOption::load_current(json);
}
QJsonValue EditableTableOption::to_json() const{
    return EditableTableBaseOption::write_current();
}

QString EditableTableOption::check_validity() const{
    return EditableTableBaseOption::check_validity();
}
void EditableTableOption::restore_defaults(){
    EditableTableBaseOption::restore_defaults();
}



ConfigWidget* EditableTableOption::make_ui(QWidget& parent){
    return new EditableTableWidget(parent, *this);
}



QWidget* make_boolean_table_cell(QWidget& parent, bool& value){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(value);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&value, box](int){
            value = box->isChecked();
        }
    );
    return widget;
}


template<typename T> QWidget* make_integer_table_cell(QWidget& parent, T& value){
    QLineEdit* box = new QLineEdit(QString::number(value), &parent);
    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&value, box](const QString& text){
            bool ok = false;
            const int current = (int)text.toLong(&ok);
            QPalette palette;
            if (ok && current >= std::numeric_limits<T>::min() && current <= std::numeric_limits<int>::max()){
                value = (T)current;
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            box->setPalette(palette);
        }
    );
    box->connect(
        box, &QLineEdit::editingFinished,
        box, [&value, box](){
            box->setText(QString::number(value));
        }
    );
    return box;
}

QWidget* make_double_table_cell(QWidget& parent, double& value, double min, double max){
    QLineEdit* box = new QLineEdit(QString::number(value), &parent);
    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&value, box, min, max](const QString& text){
            bool ok = false;
            double current = text.toDouble(&ok);
            QPalette palette;
            if (ok && current >= min && current <= max){
                value = current;
                palette.setColor(QPalette::Text, Qt::black);
            }else{
                palette.setColor(QPalette::Text, Qt::red);
            }
            box->setPalette(palette);
        }
    );
    box->connect(
        box, &QLineEdit::editingFinished,
        box, [&value, box](){
            box->setText(QString::number(value));
        }
    );
    return box;
}


template QWidget* make_integer_table_cell(QWidget& parent, uint16_t& value);
template QWidget* make_integer_table_cell(QWidget& parent, int16_t& value);

}
