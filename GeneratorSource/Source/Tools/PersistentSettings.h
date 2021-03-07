/*  Persistent Settings
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <QString>

namespace PokemonAutomation{


extern const QString VERSION;
extern const QString DISCORD;
extern const QString GITHUB_REPO;

extern const QString SETTINGS_NAME;
extern const QString CONFIG_FOLDER_NAME;
extern const QString SOURCE_FOLDER_NAME;
extern const QString LOG_FOLDER_NAME;


class PersistentSettings{
public:
    void load();
    void write() const;

private:
    void determine_paths();

public:
    QString path;
    size_t board_index = 0;

};

extern PersistentSettings settings;


}
