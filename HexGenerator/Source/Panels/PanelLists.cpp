/*  Program Tabs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonDocument>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools/PersistentSettings.h"
#include "PanelList.h"
#include "PanelLists.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

PanelLists::PanelLists(QWidget& parent, MainWindow& window)
    : QTabWidget(&parent)
{
    QString path = settings.path + CONFIG_FOLDER_NAME + "/Categories.json";

    QFile file(path);
    if (!file.open(QFile::ReadOnly)){
        QMessageBox box;
        box.critical(nullptr, "Error", "Unable to open program list: " + path);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()){
        QMessageBox box;
        box.critical(nullptr, "Error", "Invalid program list: " + path);
        return;
    }

    for (const auto& item : doc.array()){
        try{
            PanelList* tab = new PanelList(*this, window, item);
            addTab(tab, tab->display_name());
            if (tab->count() == 0){
                setTabEnabled(this->count() - 1, false);
            }
        }catch (StringException&){
            continue;
        }
    }



}


}

