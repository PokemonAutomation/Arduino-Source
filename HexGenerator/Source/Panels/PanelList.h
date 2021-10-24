/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelList_H
#define PokemonAutomation_PanelList_H

#include <memory>
#include <QListWidget>
#include "ConfigSet.h"

namespace PokemonAutomation{

class PanelList : public QListWidget{
public:
    PanelList(QWidget& parent, MainWindow& window, const QJsonValue& json);

    const QString& display_name() const{ return m_display_name; }

public slots:
    void row_selected(QListWidgetItem* item);

private:
    MainWindow& m_window;
    QString m_display_name;
    std::map<QString, std::unique_ptr<RightPanel>> m_map;
};


}
#endif
