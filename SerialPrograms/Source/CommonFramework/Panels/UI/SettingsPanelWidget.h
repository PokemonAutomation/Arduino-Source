/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SettingsPanelWidget_H
#define PokemonAutomation_SettingsPanelWidget_H

#include "PanelWidget.h"
#include "CommonFramework/Panels/SettingsPanel.h"

class QPushButton;

namespace PokemonAutomation{

class BatchWidget;


class SettingsPanelWidget : public PanelWidget{
public:
    static SettingsPanelWidget* make(
        QWidget& parent,
        SettingsPanelInstance& instance
    );

    void restore_defaults();

private:
    SettingsPanelWidget(
        QWidget& parent,
        SettingsPanelInstance& instance
    );
    void construct();
    QWidget* make_options(QWidget& parent);
    QWidget* make_actions(QWidget& parent);

private:
    friend class SettingsPanelInstance;

    BatchWidget* m_options;
    QPushButton* m_default_button;
};



}
#endif
