/*  Wallpaper Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_WallpaperWidget_H
#define PokemonAutomation_WallpaperWidget_H

#include <cstdint>
#include <functional>
#include <QColor>
#include <QFutureWatcher>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QWidget>
#include "CommonFramework/Options/WallpaperOption.h"

class QPaintEvent;

namespace PokemonAutomation{

struct WallpaperImageResult{
    QImage image;
    QString error;
};

class WallpaperWidget : public QWidget{
public:
    WallpaperWidget(QWidget* parent, std::function<void()> on_loaded);
    ~WallpaperWidget();

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
    std::function<void()> m_on_loaded;
    QFutureWatcher<WallpaperImageResult>* m_loading = nullptr;
    uint64_t m_generation = 0;
    bool m_tile = false;
    bool m_failed = false;
    QString m_error;
    QString m_path;
    QPixmap m_pixmap;
    WallpaperImageFitMode m_fit_mode = WallpaperImageFitMode::FILL;
    uint8_t m_overlay = 35;
    QColor m_surface_color;
};

}

#endif
