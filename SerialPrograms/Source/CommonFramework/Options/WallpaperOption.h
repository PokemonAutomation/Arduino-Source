/*  Wallpaper Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_WallpaperOption_H
#define PokemonAutomation_WallpaperOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/PathOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{



enum class WallpaperImageFitMode{
    FILL,
    FIT,
    STRETCH,
    TILE,
};



class WallpaperOption : public GroupOption, private ConfigOption::Listener{
public:
    ~WallpaperOption();
    WallpaperOption();

protected:
    virtual void on_config_value_changed(void* object) override;

public:
    PathOption IMAGE_PATH;
    EnumDropdownOption<WallpaperImageFitMode> IMAGE_FIT;
    SimpleIntegerOption<uint8_t> IMAGE_OVERLAY;
};





}
#endif
