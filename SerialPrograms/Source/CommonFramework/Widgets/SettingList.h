/*  UI List for all the Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SettingListUI_H
#define PokemonAutomation_SettingListUI_H

#include <QListWidget>
#include <QVBoxLayout>

namespace PokemonAutomation{


class RightPanelObject;
class MainWindow;

class SettingListUI : public QListWidget{
    Q_OBJECT

public:
    SettingListUI(MainWindow& parent);
    int text_width() const{ return m_text_width; }

public slots:
    void row_selected(QListWidgetItem* item);

private:
    MainWindow& m_parent;
    int m_text_width;
    QListWidgetItem* m_current;
    RightPanelObject* m_active_panel;
};


}
#endif

