/*  Wallpaper Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <utility>
#include <QApplication>
#include <QImageReader>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "WallpaperWidget.h"

namespace PokemonAutomation{

namespace{

WallpaperImageResult load_background_image(const QString& path){
    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()){
        return {{}, reader.errorString()};
    }
    return {std::move(image), {}};
}

QString get_theme_default_path(){
    if (QApplication::palette().color(QPalette::Active, QPalette::Window).lightness() < 128 
        || GlobalSettings::instance().THEME->get() == UiThemeMode::DARK_MODE){
        return WallpaperWidget::DEFAULT_WALLPAPER_DARK_PATH;
    }else{
        return WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_PATH;
    }   
}

bool is_default_path(const QString& path){
    return path == WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_PATH || path == WallpaperWidget::DEFAULT_WALLPAPER_DARK_PATH;
}

}

const QString WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_PATH = QString::fromStdString(RESOURCE_PATH() + "Wallpapers/light-alpha.png");
const QString WallpaperWidget::DEFAULT_WALLPAPER_DARK_PATH = QString::fromStdString(RESOURCE_PATH() + "Wallpapers/dark-alpha.png");

WallpaperWidget::WallpaperWidget(QWidget* parent)
    : QWidget(parent)
{}


void WallpaperWidget::set_appearance(
    WallpaperImageFitMode fit_mode,
    uint8_t overlay,
    const QColor& surface_color
){
    if (m_fit_mode != fit_mode){
        m_scaled_pixmap = QPixmap();
    }
    m_fit_mode = fit_mode;
    m_overlay = overlay;
    m_surface_color = surface_color;
    update();
}

void WallpaperWidget::set_image(bool enabled, const QString& path){
    if (!enabled){
        m_pixmap = QPixmap();
        m_scaled_pixmap = QPixmap();
        return;
    }

    QString requested_path = path;
    if (requested_path.isEmpty() || is_default_path(requested_path)){
        requested_path = get_theme_default_path();
    }

    if (requested_path == m_path && !m_pixmap.isNull()){
        return;
    }

    m_path = requested_path;
    m_error.clear();
    // Release the previous image before decoding its replacement.
    m_pixmap = QPixmap();
    m_scaled_pixmap = QPixmap();
    update();
    if (m_path.isEmpty()){
        return;
    }

    WallpaperImageResult result;
    try{
        result = load_background_image(requested_path);
        m_error = result.error;
    }
    catch (...){
        m_error = "Unable to decode the background image.";
    }

    if (!result.image.isNull()){
        m_pixmap = QPixmap::fromImage(std::move(result.image));
        if (m_pixmap.isNull()){
            m_error = "Unable to create the background pixmap.";
        }
    }

    update();
}

bool WallpaperWidget::active() const{
    return !m_pixmap.isNull();
}

QString WallpaperWidget::take_error(){
    return std::exchange(m_error, QString());
}

void WallpaperWidget::paintEvent(QPaintEvent*){
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    if (m_pixmap.isNull()){
        return;
    }

    if (m_fit_mode == WallpaperImageFitMode::TILE){
        painter.drawTiledPixmap(rect(), m_pixmap);
    }else{
        const qreal pixel_ratio = devicePixelRatioF();
        if (m_scaled_pixmap.isNull() || m_cached_size != size() || m_cached_pixel_ratio != pixel_ratio){
            Qt::AspectRatioMode aspect_ratio = Qt::IgnoreAspectRatio;
            if (m_fit_mode == WallpaperImageFitMode::FILL){
                aspect_ratio = Qt::KeepAspectRatioByExpanding;
            }else if (m_fit_mode == WallpaperImageFitMode::FIT){
                aspect_ratio = Qt::KeepAspectRatio;
            }
            m_scaled_pixmap = m_pixmap.scaled(size() * pixel_ratio, aspect_ratio, Qt::SmoothTransformation);
            m_scaled_pixmap.setDevicePixelRatio(pixel_ratio);
            m_cached_size = size();
            m_cached_pixel_ratio = pixel_ratio;
        }
        const QSizeF scaled_size = m_scaled_pixmap.deviceIndependentSize();
        painter.drawPixmap(QPointF(
            (width() - scaled_size.width()) / 2,
            (height() - scaled_size.height()) / 2
        ), m_scaled_pixmap);
    }

    if (m_overlay > 0){
        int alpha = static_cast<int>(m_overlay) * 255 / 100;
        QColor overlay = m_surface_color;
        overlay.setAlpha(alpha);
        painter.fillRect(rect(), overlay);
    }
}

}
