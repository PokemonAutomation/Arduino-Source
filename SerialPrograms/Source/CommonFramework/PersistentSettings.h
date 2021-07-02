/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_PersistentSettings_H
#define PokemonAutomation_PersistentSettings_H

#include <QSize>
#include <QString>
#include <QJsonObject>
//#include "Tools/StatsDatabase.h"

namespace PokemonAutomation{


class PersistentSettings{
public:
    PersistentSettings();

    void write() const;
    void read();

public:
    QString stats_file;
//    QSize window_size;
    uint32_t window_width;
    uint32_t window_height;
    bool naughty_mode;
    bool developer_mode;
    bool log_everything;

    QString resource_path;
    QString training_data;

public:
    //  Settings Panel
    QString DISCORD_WEBHOOK_ID;
    QString DISCORD_WEBHOOK_TOKEN;
    QString DISCORD_USER_ID;
    QString DISCORD_USER_SHORT_NAME;

public:
    QJsonObject panels;
};

PersistentSettings& PERSISTENT_SETTINGS();



}


#endif
