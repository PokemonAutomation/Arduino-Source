/*  UI List for all the Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SettingsListUI_H
#define PokemonAutomation_SettingsListUI_H

#include <QListWidget>
#include <QVBoxLayout>

namespace PokemonAutomation{


class MainWindow;

class SettingsListUI : public QListWidget{
    Q_OBJECT

public:
    SettingsListUI(MainWindow& parent);
    int text_width() const{ return m_text_width; }

//    virtual QSize sizeHint() const override;

public slots:
    void row_selected(QListWidgetItem* item);
    void row_changed(int row);

private:
    MainWindow& m_parent;
    int m_text_width;
};


}
#endif
