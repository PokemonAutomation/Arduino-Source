/*  Box Float Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "Common/Cpp/StringTools.h"
#include "BoxFloatWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ConfigWidget* BoxFloatOption::make_QtWidget(QWidget& parent){
    return new BoxFloatWidget(parent, *this);
}






BoxFloatWidget::~BoxFloatWidget(){
    m_value.remove_listener(*this);
}

BoxFloatWidget::BoxFloatWidget(QWidget& parent, BoxFloatOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    text->setOpenExternalLinks(true);
    layout->addWidget(text, 1);

    double x, y, width, height;
    m_value.get_all(x, y, width, height);

    QVBoxLayout* right_side = new QVBoxLayout();
    layout->addLayout(right_side, 1);

    QHBoxLayout* coordinates = new QHBoxLayout();
    right_side->addLayout(coordinates);

    m_x = new QLineEdit( this);
    m_y = new QLineEdit(this);
    m_width = new QLineEdit(this);
    m_height = new QLineEdit(this);

    coordinates->addWidget(new QLabel("X = ", this));
    coordinates->addWidget(m_x);
    coordinates->addWidget(new QLabel("Y = ", this));
    coordinates->addWidget(m_y);
    coordinates->addWidget(new QLabel("Width = ", this));
    coordinates->addWidget(m_width);
    coordinates->addWidget(new QLabel("Height = ", this));
    coordinates->addWidget(m_height);

    QHBoxLayout* array = new QHBoxLayout();
    right_side->addLayout(array);

    m_array = new QLineEdit(this);
    array->addWidget(new QLabel("Array = ", this));
    array->addWidget(m_array);

    BoxFloatWidget::update_value();

    connect(
        m_x, &QLineEdit::editingFinished,
        this, [this]{
            bool ok;
            double current = m_x->text().toDouble(&ok);

            double x, y, width, height;
            m_value.get_all(x, y, width, height);
            x = current;

            bool redraw = m_value.sanitize(x, y, width, height);
            m_value.set_all(x, y, width, height);

            if (redraw){
                update_value();
            }
        }
    );
    connect(
        m_y, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            double current = m_y->text().toDouble(&ok);

            double x, y, width, height;
            m_value.get_all(x, y, width, height);
            y = current;

            bool redraw = m_value.sanitize(x, y, width, height);
            m_value.set_all(x, y, width, height);

            if (redraw){
                update_value();
            }
        }
    );
    connect(
        m_width, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            double current = m_width->text().toDouble(&ok);

            double x, y, width, height;
            m_value.get_all(x, y, width, height);
            width = current;

            bool redraw = m_value.sanitize(x, y, width, height);
            m_value.set_all(x, y, width, height);

            if (redraw){
                update_value();
            }
        }
    );
    connect(
        m_height, &QLineEdit::editingFinished,
        this, [this](){
            bool ok;
            double current = m_height->text().toDouble(&ok);

            double x, y, width, height;
            m_value.get_all(x, y, width, height);
            height = current;

            bool redraw = m_value.sanitize(x, y, width, height);
            m_value.set_all(x, y, width, height);

            if (redraw){
                update_value();
            }
        }
    );
    connect(
        m_array, &QLineEdit::editingFinished,
        this, [this](){
            std::vector<std::string> all_coords = StringTools::split(m_array->text().toStdString(), ", ");
            if (all_coords.size() != 4){
                return;
            }

            double x = std::stod(all_coords[0]);
            double y = std::stod(all_coords[1]);
            double width = std::stod(all_coords[2]);
            double height = std::stod(all_coords[3]);

            bool redraw = m_value.sanitize(x, y, width, height);
            m_value.set_all(x, y, width, height);

            if (redraw){
                update_value();
            }
        }
    );

    value.add_listener(*this);
}


void BoxFloatWidget::update_value(){
    double x, y, width, height;
    m_value.get_all(x, y, width, height);

    std::string str_x = std::to_string(x);
    std::string str_y = std::to_string(y);
    std::string str_w = std::to_string(width);
    std::string str_h = std::to_string(height);

    m_x->setText(QString::fromStdString(str_x));
    m_y->setText(QString::fromStdString(str_y));
    m_width->setText(QString::fromStdString(str_w));
    m_height->setText(QString::fromStdString(str_h));

    std::string box_coord_string;
    box_coord_string += str_x;
    box_coord_string += ", ";
    box_coord_string += str_y;
    box_coord_string += ", ";
    box_coord_string += str_w;
    box_coord_string += ", ";
    box_coord_string += str_h;
    m_array->setText(QString::fromStdString(box_coord_string));
}
void BoxFloatWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}



}
