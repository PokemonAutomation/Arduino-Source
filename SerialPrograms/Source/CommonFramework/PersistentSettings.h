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
#include "Integrations/DiscordSettingsOption.h"

namespace PokemonAutomation{


class PersistentSettings{
public:
    PersistentSettings();

    void write() const;
    void read();

public:
    QJsonObject panels;
};

PersistentSettings& PERSISTENT_SETTINGS();



}


#endif
