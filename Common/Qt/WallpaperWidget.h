/*  Wallpaper Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_WallpaperWidget_H
#define PokemonAutomation_WallpaperWidget_H

#include <cstdint>
#include "CommonFramework/Options/WallpaperOption.h"

class QPaintEvent;

namespace PokemonAutomation{

struct WallpaperImageResult{
    QImage image;
    QString error;
};

class WallpaperWidget : public QWidget{
public:
    static const QString DEFAULT_WALLPAPER_LIGHT_PATH;
    static const QString DEFAULT_WALLPAPER_DARK_PATH;

    WallpaperWidget(QWidget* parent);

    void set_appearance(
        WallpaperImageFitMode fit_mode,
        uint8_t overlay,
        const QColor& surface_color
    );
    void set_image(bool enabled, const QString& path);
    bool active() const;
    QString take_error();

protected:
    virtual void paintEvent(QPaintEvent*) override;

private:
    QString m_error;
    QString m_path;
    QPixmap m_pixmap;
    QPixmap m_scaled_pixmap;
    QSize m_cached_size;
    qreal m_cached_pixel_ratio = 0;
    WallpaperImageFitMode m_fit_mode = WallpaperImageFitMode::FILL;
    uint8_t m_overlay = 35;
    QColor m_surface_color;
};

}

#endif
