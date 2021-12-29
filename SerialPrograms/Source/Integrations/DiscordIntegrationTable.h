/*  Discord Integration Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationTable_H
#define PokemonAutomation_DiscordIntegrationTable_H

#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordIntegrationChannel : public EditableTableRow{
public:
    DiscordIntegrationChannel();

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_enabled_box(QWidget& parent);
    QWidget* make_label_box(QWidget& parent);
    QWidget* make_ping_box(QWidget& parent);
    QWidget* make_tags_box(QWidget& parent);
    QWidget* make_commands_box(QWidget& parent);
    QWidget* make_channel_box(QWidget& parent);

public:
    bool enabled;
    QString label;
    bool ping;
    std::vector<QString> tags;
    bool allow_commands;
    QString channel_id;
};

class DiscordIntegrationTableFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};


class DiscordIntegrationTable : public DiscordIntegrationTableFactory, public EditableTableOption{
public:
    DiscordIntegrationTable();

    virtual void load_json(const QJsonValue& json) override;

    size_t size() const;
    const DiscordIntegrationChannel& operator[](size_t index) const;

    std::vector<QString> command_channels() const;
};




}
}
#endif
