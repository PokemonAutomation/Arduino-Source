/*  Wallpaper Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_WallpaperWidget_H
#define PokemonAutomation_WallpaperWidget_H

#include <cstdint>
#include <memory>
#include <QImage>
#include <QWidget>
#include "CommonFramework/Options/WallpaperOption.h"

class QPaintEvent;
class QSvgRenderer;

namespace PokemonAutomation{

struct WallpaperImageResult{
    QImage image;
    QString error;
};

// Sets up the main window's background wallpaper. Supports the light and dark theme.
class WallpaperWidget : public QWidget{
public:
    // Built-in wallpapers in RESOURCE_PATH()/Wallpapers/. The SVG version is used when
    // it exists. The PNG version is the fallback for resource folders without it.
    static const QString DEFAULT_WALLPAPER_LIGHT_PNG_PATH;
    static const QString DEFAULT_WALLPAPER_DARK_PNG_PATH;
    static const QString DEFAULT_WALLPAPER_LIGHT_SVG_PATH;
    static const QString DEFAULT_WALLPAPER_DARK_SVG_PATH;

    WallpaperWidget(QWidget* parent);
    ~WallpaperWidget();

    void set_appearance(
        WallpaperImageFitMode fit_mode,
        uint8_t overlay,
        const QColor& surface_color
    );

    // Enable or disable the wallpaper.
    // If enabled, set the wallpaper from an image at `path`.
    // If the path is one of the built-in wallpaper path, will overwrite the path
    // with the path of a built-in wallaper that matches the current color theme.
    void set_image(bool enabled, const QString& path);
    // Whether the wallpaer is enabled.
    bool active() const;
    QString take_error();

protected:
    virtual void paintEvent(QPaintEvent*) override;

private:
    // Load the SVG file at `path` into `m_svg`. Set `m_error` if the file is missing
    // or is not valid SVG.
    void load_svg(const QString& path);

    // Draw `m_svg` into a new pixmap at the screen's physical resolution `pixel_ratio`.
    // For FILL, FIT and STRETCH the pixmap covers the whole widget, with the SVG placed
    // the same way a raster image would be. For TILE it is one tile at the SVG's own
    // size (its width and height attributes).
    QPixmap render_svg(qreal pixel_ratio);

private:
    QString m_error;
    QString m_path;
    // Holds the wallpaper: `m_pixmap` for a raster image, `m_svg` for an SVG image.
    // At most one of them is set.
    QPixmap m_pixmap;
    std::unique_ptr<QSvgRenderer> m_svg;
    // The wallpaper drawn at the current widget size and pixel ratio, kept to avoid
    // scaling or rendering it again on every repaint.
    QPixmap m_scaled_pixmap;
    QSize m_cached_size;
    qreal m_cached_pixel_ratio = 0;
    WallpaperImageFitMode m_fit_mode = WallpaperImageFitMode::FILL;
    uint8_t m_overlay = 35;
    QColor m_surface_color;
};

}

#endif
