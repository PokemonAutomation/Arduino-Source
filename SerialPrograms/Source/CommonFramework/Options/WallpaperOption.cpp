/*  Wallpaper Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "WallpaperOption.h"

namespace PokemonAutomation{



WallpaperOption::~WallpaperOption(){
    IMAGE_OVERLAY.remove_listener(*this);
    IMAGE_FIT.remove_listener(*this);
    IMAGE_PATH.remove_listener(*this);
}
WallpaperOption::WallpaperOption()
    : GroupOption(
        "Wallpaper",
        LockMode::UNLOCK_WHILE_RUNNING,
        IS_BETA_VERSION
            ? EnableMode::DEFAULT_ENABLED
            : EnableMode::DEFAULT_DISABLED
    )
    , IMAGE_PATH(
        "<b>Wallpaper Path:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "",
        "Images (*.png *.jpg *.jpeg *.bmp *.webp);;All Files (*)",
        "Select a PNG, JPEG, BMP, or WebP image"
    )
    , IMAGE_FIT(
        "<b>Wallpaper Image Fit:</b>",
        {
            {WallpaperImageFitMode::FILL, "fill", "Fill (crop to window)"},
            {WallpaperImageFitMode::FIT, "fit", "Fit (show entire image)"},
            {WallpaperImageFitMode::STRETCH, "stretch", "Stretch"},
            {WallpaperImageFitMode::TILE, "tile", "Tile"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        WallpaperImageFitMode::FILL
    )
    , IMAGE_OVERLAY(
        "<b>Background Overlay (%):</b><br>Increase this to improve text readability over the image.",
        LockMode::UNLOCK_WHILE_RUNNING,
        35, 0, 100
    )
{
    PA_ADD_OPTION(IMAGE_PATH);
    PA_ADD_OPTION(IMAGE_FIT);
    PA_ADD_OPTION(IMAGE_OVERLAY);

    IMAGE_PATH.add_listener(*this);
    IMAGE_FIT.add_listener(*this);
    IMAGE_OVERLAY.add_listener(*this);
}


void WallpaperOption::on_config_value_changed(void* object){
    ConfigOption::report_value_changed(this);
}






}
