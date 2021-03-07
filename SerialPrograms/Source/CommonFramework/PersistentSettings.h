/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QSize>
#include <QString>
#include <QJsonObject>

#ifndef PokemonAutomation_PersistentSettings_H
#define PokemonAutomation_PersistentSettings_H

namespace PokemonAutomation{


class PersistentSettings{
public:
    void write() const;
    void read();

public:
    QString config_path = "GeneratorConfig";
    QString source_path = "DeviceSource";
    QSize window_size = QSize(960, 540);
    bool naughty_mode = false;
    bool developer_mode = false;
    bool log_everything = false;

    QJsonObject settings;
    QJsonObject programs;
};

extern PersistentSettings settings;



}


#endif
