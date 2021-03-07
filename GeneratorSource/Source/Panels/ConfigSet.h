/*  Parent Class for all Configuration Files
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_ConfigSet_H
#define PokemonAutomation_ConfigSet_H

#include <QObject>
#include <QJsonArray>
#include "RightPanel.h"

namespace PokemonAutomation{


class ConfigSet : public RightPanel{
public:
    static const QString JSON_CONFIG_NAME;
    static const QString JSON_CONFIG_PATH;
    static const QString JSON_DESCRIPTION;
    static const QString JSON_OPTIONS;

public:
    ConfigSet(const QJsonObject& obj);

    const QString& description() const{ return m_description; }

    virtual bool is_valid() const{ return true; }
    virtual void restore_defaults(){}

    QJsonDocument to_json() const;
    std::string to_cfile() const;

    QString save_json() const;
    QString save_cfile() const;

    virtual QWidget* make_ui(MainWindow& parent) override;

protected:
    virtual QWidget* make_options_body(QWidget& parent) = 0;
    virtual QJsonArray options_json() const{ return QJsonArray(); }
    virtual std::string options_cpp() const{ return ""; }
    virtual void from_json(const QJsonDocument&){}

private:
    QString m_path;
    QString m_description;
};


}
#endif
