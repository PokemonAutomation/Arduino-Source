/*  Program Tab
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramTab_H
#define PokemonAutomation_ProgramTab_H

#include <memory>
#include <QListWidget>
#include "ConfigSet.h"

namespace PokemonAutomation{

class ProgramTab : public QListWidget{
public:
    ProgramTab(QWidget& parent, MainWindow& window, const QJsonValue& json);

    const QString& display_name() const{ return m_display_name; }
    size_t items() const{ return m_list.size(); }

public slots:
    void row_selected(QListWidgetItem* item);

private:
    MainWindow& m_window;
    QString m_display_name;
    std::vector<std::unique_ptr<RightPanel>> m_list;
    std::map<QString, RightPanel*> m_map;
};


}
#endif
