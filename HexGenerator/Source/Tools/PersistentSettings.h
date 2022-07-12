/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>

namespace PokemonAutomation{


extern const std::string VERSION;
extern const std::string DISCORD;
extern const std::string GITHUB_REPO;

extern const std::string SETTINGS_NAME;
extern const std::string CONFIG_FOLDER_NAME;
extern const std::string SOURCE_FOLDER_NAME;
extern const std::string LOG_FOLDER_NAME;


class PersistentSettings{
public:
    void load();
    void write() const;

private:
    void determine_paths();

public:
    std::string path;
    size_t board_index = 0;

};

extern PersistentSettings settings;


}
