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
    QSize window_size;
    bool naughty_mode;
    bool developer_mode;
    std::atomic<bool> log_everything;

    QString resource_path;
//    StatSet stat_sets;

    QJsonObject settings;
    QJsonObject programs;
};

PersistentSettings& PERSISTENT_SETTINGS();



}


#endif
