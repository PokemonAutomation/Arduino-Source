/*  Custom Path Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "PokemonLA_CustomPathTableWidget.h"

namespace PokemonAutomation{

template class RegisterConfigWidget<NintendoSwitch::PokemonLA::CustomPathTableWidget>;

namespace NintendoSwitch{
namespace PokemonLA{


CustomPathTableWidget::CustomPathTableWidget(QWidget& parent, CustomPathTableFromJubilife& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    // m_table_widget is class EditableTableWidget : public EditableTableBaseWidget, public ConfigWidget
    // EditableTableBaseWidget inherits QWidget.
    // Since it's a QWidget, we don't need to care about its memory ownership after its parent is set (as `this`).

    m_travel_location = value.TRAVEL_LOCATION.make_QtWidget(*this);
    m_table_widget = value.PATH.make_QtWidget(*this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&m_travel_location->widget());
    layout->addWidget(&m_table_widget->widget());

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(button_layout);
    auto load_button = new QPushButton("Load Path", this);
    button_layout->addWidget(load_button, 1);
    auto save_button = new QPushButton("Save Path", this);
    button_layout->addWidget(save_button, 1);
    button_layout->addStretch(2);

    connect(load_button,  &QPushButton::clicked, this, [&value, this](bool){
        std::string path = QFileDialog::getOpenFileName(this, tr("Open option file"), ".", "*.json").toStdString();
        std::cout << "Load CustomPathTableFromJubilife from " << path << std::endl;
        if (path.empty()){
            return;
        }
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);
        JsonValue& obj = root.get_value_throw("CUSTOM_PATH_TABLE", path);
        value.load_json(obj);
        if (m_table_widget == nullptr){
            QMessageBox box;
            box.critical(nullptr, "Error", "Internal code error, cannot convert to EditableTableBaseWidget.");
            return;
        }
//            m_travel_location->update();
//            m_table_widget->update();
    });

    connect(save_button,  &QPushButton::clicked, this, [&value, this](bool){
        std::string path = QFileDialog::getSaveFileName(this, tr("Open option file"), ".", "*.json").toStdString();
        std::cout << "Save CustomPathTableFromJubilife from " << path << std::endl;
        if (path.size() > 0){
            try{
                JsonObject root;
                root["CUSTOM_PATH_TABLE"] = value.to_json();
                root.dump(path);
            }catch (FileException&){
                QMessageBox box;
                box.critical(nullptr, "Error", QString::fromStdString("Failed to save to file: " + path));
                return;
            }
        }
    });
}



}
}
}
