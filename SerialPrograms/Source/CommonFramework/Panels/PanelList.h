/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelList_H
#define PokemonAutomation_PanelList_H

#include <memory>
#include <QListWidget>
#include "CommonFramework/Panels/Panel.h"

namespace PokemonAutomation{


class PanelList : public QListWidget{
public:
    static const QString JSON_PROGRAM_PANEL;

public:
    PanelList(QTabWidget& parent, QString label, PanelListener& listener);

    const QString& label() const{ return m_label; }
    size_t items() const{ return m_panels.size(); }

    void set_panel(const QString& panel_name);

protected:
    void add_divider(QString label);

    void handle_panel_clicked(const QString text);

    template <typename Descriptor, typename Instance, class... Args>
    void add_settings(Args&&... args){
        add_program<Descriptor, Instance>(std::forward<Args>(args)...);
        //  Need to force initialize a settings panel so that it loads and
        //  updates the globals from serialization.
        m_panels.back().second->make_panel()->from_json();
    }

    template <typename Descriptor, typename Instance, class... Args>
    void add_program(Args&&... args){
        std::unique_ptr<PanelDescriptor> panel(
            new PanelDescriptorWrapper<Descriptor, Instance>(std::forward<Args>(args)...)
        );
        m_panels.emplace_back(panel->display_name(), std::move(panel));
    }
    void finish_panel_setup();

protected:
    QString m_label;
    PanelListener& m_listener;
    std::vector<std::pair<QString, std::unique_ptr<PanelDescriptor>>> m_panels;
private:
    std::map<QString, const PanelDescriptor*> m_panel_map;
};




}
#endif
