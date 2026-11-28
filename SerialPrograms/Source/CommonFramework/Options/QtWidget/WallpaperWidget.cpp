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
#include <QSvgRenderer>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Strings/StringTools.h"
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

// Return true if `path` names an SVG file, judged by its extension
bool is_svg_path(const QString& path){
    const std::string extension = Filesystem::Path(path.toStdString()).extension().string();
    return StringTools::ends_with_ci(extension, ".svg")
        || StringTools::ends_with_ci(extension, ".svgz");
}

// Return the built-in wallpaper for the current theme: the SVG version if the
// resource folder has it, otherwise the PNG version.
QString get_theme_default_path(){
    const bool dark_mode = QApplication::palette().color(QPalette::Active, QPalette::Window).lightness() < 128
        || GlobalSettings::instance().THEME->get() == UiThemeMode::DARK_MODE;
    const QString& svg_path = dark_mode
        ? WallpaperWidget::DEFAULT_WALLPAPER_DARK_SVG_PATH
        : WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_SVG_PATH;
    if (Filesystem::exists(Filesystem::Path(svg_path.toStdString()))){
        return svg_path;
    }
    return dark_mode
        ? WallpaperWidget::DEFAULT_WALLPAPER_DARK_PNG_PATH
        : WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_PNG_PATH;
}

bool is_default_path(const QString& path){
    return path == WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_PNG_PATH
        || path == WallpaperWidget::DEFAULT_WALLPAPER_DARK_PNG_PATH
        || path == WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_SVG_PATH
        || path == WallpaperWidget::DEFAULT_WALLPAPER_DARK_SVG_PATH;
}

// Return the rectangle, in widget coordinates, that a wallpaper of `source_size`
// covers inside a widget of `widget_size` for `fit_mode` (FILL, FIT or STRETCH).
// The rectangle is centered. For FILL it can be larger than the widget, and the parts
// outside the widget are cropped. E.g. a 1088x992 wallpaper in a 1600x1000 widget:
//   FILL:    1600x1459 at (0, -229)
//   FIT:     1097x1000 at (252, 0)
//   STRETCH: 1600x1000 at (0, 0)
QRectF fitted_rect(const QSizeF& source_size, const QSizeF& widget_size, WallpaperImageFitMode fit_mode){
    Qt::AspectRatioMode aspect_ratio = Qt::IgnoreAspectRatio;
    if (fit_mode == WallpaperImageFitMode::FILL){
        aspect_ratio = Qt::KeepAspectRatioByExpanding;
    }else if (fit_mode == WallpaperImageFitMode::FIT){
        aspect_ratio = Qt::KeepAspectRatio;
    }
    const QSizeF size = source_size.scaled(widget_size, aspect_ratio);
    return QRectF(
        QPointF(
            (widget_size.width() - size.width()) / 2,
            (widget_size.height() - size.height()) / 2
        ),
        size
    );
}

}

const QString WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_PNG_PATH = QString::fromStdString(RESOURCE_PATH() + "Wallpapers/light-alpha.png");
const QString WallpaperWidget::DEFAULT_WALLPAPER_DARK_PNG_PATH = QString::fromStdString(RESOURCE_PATH() + "Wallpapers/dark-alpha.png");
const QString WallpaperWidget::DEFAULT_WALLPAPER_LIGHT_SVG_PATH = QString::fromStdString(RESOURCE_PATH() + "Wallpapers/light-alpha.svg");
const QString WallpaperWidget::DEFAULT_WALLPAPER_DARK_SVG_PATH = QString::fromStdString(RESOURCE_PATH() + "Wallpapers/dark-alpha.svg");

WallpaperWidget::WallpaperWidget(QWidget* parent)
    : QWidget(parent)
{}
WallpaperWidget::~WallpaperWidget() = default;


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
        m_svg.reset();
        m_scaled_pixmap = QPixmap();
        return;
    }

    QString requested_path = path;
    if (requested_path.isEmpty() || is_default_path(requested_path)){
        requested_path = get_theme_default_path();
    }

    if (requested_path == m_path && active()){
        return;
    }

    m_path = requested_path;
    m_error.clear();
    // Release the previous image before decoding its replacement.
    m_pixmap = QPixmap();
    m_svg.reset();
    m_scaled_pixmap = QPixmap();
    update();
    if (m_path.isEmpty()){
        return;
    }

    if (is_svg_path(requested_path)){
        load_svg(requested_path);
        update();
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

void WallpaperWidget::load_svg(const QString& path){
    try{
        auto renderer = std::make_unique<QSvgRenderer>(path);
        if (renderer->isValid()){
            m_svg = std::move(renderer);
        }else{
            m_error = "Unable to read the SVG image.";
        }
    }
    catch (...){
        m_error = "Unable to read the SVG image.";
    }
}

QPixmap WallpaperWidget::render_svg(qreal pixel_ratio){
    const QSizeF svg_size = m_svg->defaultSize();
    QSizeF pixmap_size;
    QRectF target;
    if (m_fit_mode == WallpaperImageFitMode::TILE){
        pixmap_size = svg_size;
        target = QRectF(QPointF(0, 0), svg_size);
    }else{
        pixmap_size = size();
        target = fitted_rect(svg_size, size(), m_fit_mode);
    }
    if (svg_size.isEmpty() || pixmap_size.isEmpty()){
        return QPixmap();
    }

    // e.g. a 1728x1117 window on a 2x Retina screen gives a 3456x2234 pixmap.
    QPixmap pixmap((pixmap_size * pixel_ratio).toSize());
    pixmap.setDevicePixelRatio(pixel_ratio);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_svg->render(&painter, target);
    return pixmap;
}

bool WallpaperWidget::active() const{
    return !m_pixmap.isNull() || m_svg != nullptr;
}

QString WallpaperWidget::take_error(){
    return std::exchange(m_error, QString());
}

void WallpaperWidget::paintEvent(QPaintEvent*){
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);

    if (!active()){
        return;
    }

    if (m_svg){
        // Render the SVG again whenever the widget size or screen scale changes, so
        // it is always drawn at full resolution.
        const qreal pixel_ratio = devicePixelRatioF();
        if (m_scaled_pixmap.isNull() || m_cached_size != size() || m_cached_pixel_ratio != pixel_ratio){
            m_scaled_pixmap = render_svg(pixel_ratio);
            m_cached_size = size();
            m_cached_pixel_ratio = pixel_ratio;
        }
        // The pixmap is null when there is nothing to draw, e.g. at zero widget size.
        if (!m_scaled_pixmap.isNull()){
            if (m_fit_mode == WallpaperImageFitMode::TILE){
                painter.drawTiledPixmap(rect(), m_scaled_pixmap);
            }else{
                painter.drawPixmap(QPointF(0, 0), m_scaled_pixmap);
            }
        }
    }else if (m_fit_mode == WallpaperImageFitMode::TILE){
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
