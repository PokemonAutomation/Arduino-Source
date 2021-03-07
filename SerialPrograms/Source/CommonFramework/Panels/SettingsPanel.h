/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SettingsPanel_H
#define PokemonAutomation_SettingsPanel_H

#include <memory>
#include <QPushButton>
#include "CommonFramework/Options/ConfigOption.h"
#include "RightPanel.h"

namespace PokemonAutomation{


class SettingsPanel : public RightPanel{
public:
    using RightPanel::RightPanel;

    void from_json(const QJsonValue& json);
    virtual QJsonValue to_json() const override;

    virtual QWidget* make_ui(MainWindow& window) override;

protected:
    friend class SettingsPanelUI;
    std::vector<std::pair<QString, std::unique_ptr<ConfigOption>>> m_options;
};



class SettingsPanelUI : public RightPanelUI{
    friend class SettingsPanel;

public:
    SettingsPanelUI(SettingsPanel& factory);
    virtual void make_body(QWidget& parent, QVBoxLayout& layout) override;

    void restore_defaults();

private:
    SettingsPanel& m_factory;
    std::vector<ConfigOptionUI*> m_options;
    QPushButton* m_default_button;
};


}
#endif


