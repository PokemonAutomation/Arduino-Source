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
#include "Panel.h"

namespace PokemonAutomation{



class SettingsPanelInstance : public PanelInstance{
public:
    using PanelInstance::PanelInstance;

    virtual QWidget* make_widget(QWidget& parent, PanelListener& listener) override;

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

protected:
    friend class SettingsPanelWidget;

    std::vector<std::pair<QString, std::unique_ptr<ConfigOption>>> m_options;
};



class SettingsPanelWidget : public PanelWidget{
public:
    static SettingsPanelWidget* make(
        QWidget& parent,
        SettingsPanelInstance& instance,
        PanelListener& listener
    );

    void restore_defaults();

private:
    SettingsPanelWidget(
        QWidget& parent,
        SettingsPanelInstance& instance,
        PanelListener& listener
    );
    void construct();
    QWidget* make_options(QWidget& parent);
    QWidget* make_actions(QWidget& parent);

private:
    friend class SettingsPanelInstance;

    std::vector<ConfigOptionUI*> m_options;
    QPushButton* m_default_button;
};




}
#endif


