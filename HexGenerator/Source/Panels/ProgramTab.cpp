/*  Program Tab
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QTextStream>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools/PersistentSettings.h"
#include "UI/MainWindow.h"
#include "JsonSettings.h"
#include "JsonProgram.h"
#include "ProgramTab.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

ProgramTab::ProgramTab(QWidget& parent, MainWindow& window, const QJsonValue& json)
    : QListWidget(&parent)
    , m_window(window)
{
    if (!json.isObject()){
        PA_THROW_ParseException("Expected an object.");
    }
    const QJsonObject& category = json.toObject();

    QString category_name;
    if (!json_get_string(category_name, category, "Name")){
        PA_THROW_ParseException("Expected string field: Name");
    }

    if (!json_get_string(m_display_name, category, "Display")){
        PA_THROW_ParseException("Expected string field: Display");
    }

    //  Populate Settings
    std::vector<QString> settings_list;
    for (const auto& item : json_get_array_nothrow(category, "Settings")){
        if (!item.isString()){
            PA_THROW_ParseException("Expected string field: Settings");
        }
        QString setting = item.toString();
//        settings.emplace_back(setting.toString());
//        m_list.emplace_back();
//        cout << setting.toUtf8().data() << endl;
        try{
            QString path = settings.path + CONFIG_FOLDER_NAME + "/" + category_name + "/" + setting + ".json";
            std::unique_ptr<RightPanel> config(new Settings_JsonFile(category_name, path));
            m_list.emplace_back(std::move(config));
            const QString& name = m_list.back()->name();
            if (!m_map.emplace(name, m_list.back().get()).second){
                PA_THROW_ParseException("Duplicate: Program name");
            }
            addItem(name);
        }catch (const StringException& e){
            cout << "Error: " << e.message() << endl;
        }
    }

    //  Populate Programs
    QString path = settings.path + SOURCE_FOLDER_NAME + "/" + category_name + "/ProgramList.txt";
    QFile file(path);
    if (file.open(QFile::ReadOnly)){
        cout << "File = " << path.toUtf8().data() << endl;
        QTextStream stream(&file);
        while (!stream.atEnd()){
            QString line = stream.readLine();
            if (line.isEmpty()){
                continue;
            }
            cout << "Open: " << line.toUtf8().data() << endl;
            try{
                QString path = settings.path + CONFIG_FOLDER_NAME + "/" + category_name + "/" + line + ".json";
                std::unique_ptr<RightPanel> config(new Program_JsonFile(category_name, path));
                m_list.emplace_back(std::move(config));
                const QString& name = m_list.back()->name();
                if (!m_map.emplace(name, m_list.back().get()).second){
                    PA_THROW_StringException("Duplicate: Program name");
                }
                addItem(name);
            }catch (const StringException& e){
                cout << "Error: " << e.message() << endl;
            }
        }
        file.close();
    }


    connect(this, &QListWidget::itemClicked, this, &ProgramTab::row_selected);
}

void ProgramTab::row_selected(QListWidgetItem* item){
    auto iter = m_map.find(item->text());
    if (iter == m_map.end()){
//        std::cout << item->text().toUtf8().data() << std::endl;
        PA_THROW_StringException("Invalid program name: " + item->text());
    }

    m_window.change_panel(*iter->second);
}


}
