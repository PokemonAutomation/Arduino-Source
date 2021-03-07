/*  List of all Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "JsonSettings.h"
#include "JsonProgram.h"
#include "Tools/PersistentSettings.h"
#include "PanelList.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



const std::vector<std::unique_ptr<ConfigSet>>& SETTINGS_LIST(){
    static std::vector<std::unique_ptr<ConfigSet>> list;
    if (!list.empty()){
        return list;
    }

    QString path = settings.path + CONFIG_FOLDER_NAME + "/SettingsList.txt";
    QFile file(path);
    if (!file.open(QFile::ReadOnly)){
//        QMessageBox box;
//        box.critical(nullptr, "Error", "Unable to open settings list: " + settings_path);
        return list;
    }
    cout << "File = " << path.toUtf8().data() << endl;

    QTextStream stream(&file);
    while (!stream.atEnd()){
        QString line = stream.readLine();
        if (line.isEmpty()){
            continue;
        }
        cout << "Open: " << line.toUtf8().data() << endl;
        try{
            QString path = settings.path + CONFIG_FOLDER_NAME + "/" + line + ".json";
            list.emplace_back(new Settings_JsonFile(path));
        }catch (const StringException& str){
            cout << "Error: " << str.message().toUtf8().data() << endl;
        }
    }
    file.close();

    return list;
}
const std::map<QString, ConfigSet*>& SETTINGS_MAP(){
    static std::map<QString, ConfigSet*> map;
    if (!map.empty()){
        return map;
    }
    for (const auto& program : SETTINGS_LIST()){
        auto ret = map.emplace(program->name(), program.get());
        if (!ret.second){
            throw StringException("Duplicate program name: " + program->name());
        }
    }
    return map;
}


const std::vector<std::unique_ptr<Program>>& PROGRAM_LIST(){
    static std::vector<std::unique_ptr<Program>> list;
    if (!list.empty()){
        return list;
    }

    QString path = settings.path + CONFIG_FOLDER_NAME + "/ProgramList.txt";
    QFile file(path);
    if (!file.open(QFile::ReadOnly)){
//        QMessageBox box;
//        box.critical(nullptr, "Error", "Unable to open programs list: " + settings_path);
        return list;
    }
    cout << "File = " << path.toUtf8().data() << endl;

    QTextStream stream(&file);
    while (!stream.atEnd()){
        QString line = stream.readLine();
        if (line.isEmpty()){
            continue;
        }
        cout << "Open: " << line.toUtf8().data() << endl;
        try{
            QString path = settings.path + CONFIG_FOLDER_NAME + "/" + line + ".json";
            list.emplace_back(new Program_JsonFile(path));
        }catch (const StringException& str){
            cout << "Error: " << str.message().toUtf8().data() << endl;
        }
    }
    file.close();

    return list;
}
const std::map<QString, Program*>& PROGRAM_MAP(){
    static std::map<QString, Program*> map;
    if (map.empty()){
        for (const auto& program : PROGRAM_LIST()){
            auto ret = map.emplace(program->name(), program.get());
            if (!ret.second){
                throw StringException("Duplicate program name: " + program->name());
            }
        }
    }
    return map;
}



}


