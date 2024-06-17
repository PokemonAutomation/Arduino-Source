/*  Program Tabs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Tools/PersistentSettings.h"
#include "PanelList.h"
#include "PanelLists.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace HexGenerator{

PanelLists::PanelLists(QWidget& parent, MainWindow& window)
    : QTabWidget(&parent)
{
    std::string path = settings.path + CONFIG_FOLDER_NAME + "/Categories.json";

    JsonValue json;
    try{
        json = load_json_file(path);
        for (const auto& item : json.to_array_throw(path)){
            try{
                PanelList* tab = new PanelList(*this, window, item);
                addTab(tab, QString::fromStdString(tab->display_name()));
                if (tab->count() == 0){
                    setTabEnabled(this->count() - 1, false);
                }
            }catch (Exception&){
                continue;
            }
        }
    }catch (ParseException&){
        QMessageBox box;
        box.critical(nullptr, "Error", "Unable to open program list: " + QString::fromStdString(path));
        return;
    }




}


}
}

