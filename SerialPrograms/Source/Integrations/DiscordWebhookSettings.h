/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordWebhookSettings_H
#define PokemonAutomation_DiscordWebhookSettings_H

#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordWebhookUrl : public EditableTableRow{
public:
    DiscordWebhookUrl();

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_enabled_box(QWidget& parent);
    QWidget* make_label_box(QWidget& parent);
    QWidget* make_ping_box(QWidget& parent);
    QWidget* make_tags_box(QWidget& parent);
    QWidget* make_url_box(QWidget& parent);

public:
    bool enabled;
    QString label;
    bool ping;
    std::vector<QString> tags;
    QString url;
};

class DiscordWebhookUrlsFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};


class DiscordWebhookSettingsOption : public GroupOption{
public:
    DiscordWebhookSettingsOption();

private:
    DiscordWebhookUrlsFactory m_factory;

public:
    EditableTableOption urls;
};



}
}
#endif
