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
#include "Tools/StatsDatabase.h"

namespace PokemonAutomation{


class PersistentSettings{
public:
    PersistentSettings()
        : log_everything(false)
    {}

    void write() const;
    void read();

public:
//    QString config_path = "GeneratorConfig";
//    QString source_path = "DeviceSource";
    QString stats_file = "PA-Stats.txt";
    QSize window_size = QSize(960, 540);
    bool naughty_mode = false;
    bool developer_mode = false;
    std::atomic<bool> log_everything;

    StatSet stat_sets;

    QJsonObject settings;
    QJsonObject programs;
};

extern PersistentSettings settings;



}


#endif
