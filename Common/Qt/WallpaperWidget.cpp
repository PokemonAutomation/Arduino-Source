/*  Wallpaper Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <algorithm>
#include <cmath>
#include <exception>
#include <memory>
#include <utility>
#include <QImageReader>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include "WallpaperWidget.h"

namespace PokemonAutomation{

namespace{

WallpaperImageResult load_background_image(const QString& path, QSize target_size, bool tile){
    QImageReader reader(path);
    reader.setAutoTransform(true);
    // Sprite loaders disable QImageReader's application-wide allocation limit.
    // Might want to reject overly large images.
    constexpr qint64 MAX_DISPLAY_PIXELS = 8 * 1024 * 1024; // 4k image size.
    const QSize dimensions = reader.size();
    if (!dimensions.isValid() || dimensions.isEmpty()){
        return {{}, "Unable to determine the image dimensions safely."};
    }
    const qint64 pixels = static_cast<qint64>(dimensions.width()) * dimensions.height();
    if (!tile){
        const qreal scale = std::min({
            qreal(1),
            std::max(qreal(target_size.width()) / dimensions.width(),
                     qreal(target_size.height()) / dimensions.height()),
            std::sqrt(qreal(MAX_DISPLAY_PIXELS) / pixels)
        });
        const QSize scaled_size(
            std::max(1, static_cast<int>(dimensions.width() * scale)),
            std::max(1, static_cast<int>(dimensions.height() * scale))
        );
        if (scaled_size != dimensions){
            reader.setScaledSize(scaled_size);
        }
    }
    QImage image = reader.read();
    if (image.isNull()){
        return {{}, reader.errorString()};
    }
    return {std::move(image), {}};
}

QThreadPool& background_image_pool(){
    // Serialize decoding so rapid path changes cannot multiply peak memory.
    static const auto pool = []{
        auto result = std::make_unique<QThreadPool>();
        result->setMaxThreadCount(1);
        return result;
    }();
    return *pool;
}

}

WallpaperWidget::WallpaperWidget(QWidget* parent, std::function<void()> on_loaded)
    : QWidget(parent)
    , m_on_loaded(std::move(on_loaded))
{}

WallpaperWidget::~WallpaperWidget(){
    if (m_loading){
        m_loading->cancel();
    }
}

void WallpaperWidget::set_appearance(
    WallpaperImageFitMode fit_mode,
    uint8_t overlay,
    const QColor& surface_color
){
    m_fit_mode = fit_mode;
    m_overlay = overlay;
    m_surface_color = surface_color;
    update();
}

void WallpaperWidget::set_image(bool enabled, const QString& path){
    const QString requested_path = enabled ? path : QString();
    const bool tile = m_fit_mode == WallpaperImageFitMode::TILE;
    if (requested_path == m_path && (requested_path.isEmpty() || m_failed || tile == m_tile)){
        return;
    }
    ++m_generation;
    if (m_loading){
        m_loading->cancel();
        m_loading = nullptr;
    }
    m_path = requested_path;
    m_tile = tile;
    m_failed = false;
    m_error.clear();
    // Release the previous image before decoding its replacement.
    m_pixmap = QPixmap();
    update();
    if (m_path.isEmpty()){
        return;
    }

    const QSize target_size = screen()
        ? screen()->size() * screen()->devicePixelRatio()
        : size();
    auto promise = std::make_shared<QPromise<WallpaperImageResult>>();
    promise->start();
    auto* watcher = new QFutureWatcher<WallpaperImageResult>(this);
    m_loading = watcher;
    const uint64_t generation = m_generation;
    connect(watcher, &QFutureWatcher<WallpaperImageResult>::finished,
        this, [this, watcher, generation]{
            watcher->deleteLater();
            if (generation != m_generation || watcher->isCanceled()){
                return;
            }
            m_loading = nullptr;
            WallpaperImageResult result;
            try{
                result = watcher->result();
            }catch (...){
                result.error = QStringLiteral("Unable to decode the background image.");
            }
            m_error = std::move(result.error);
            if (!result.image.isNull()){
                m_pixmap = QPixmap::fromImage(std::move(result.image));
                if (m_pixmap.isNull()){
                    m_error = "Unable to create the background pixmap.";
                }
            }
            m_failed = m_pixmap.isNull();
            update();
            m_on_loaded();
        }
    );
    watcher->setFuture(promise->future());
    background_image_pool().start([promise, requested_path, target_size, tile]{
        try{
            if (!promise->isCanceled()){
                auto result = load_background_image(requested_path, target_size, tile);
                if (!promise->isCanceled()){
                    promise->addResult(std::move(result));
                }
            }
        }catch (...){
            promise->setException(std::current_exception());
        }
        promise->finish();
    });
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

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    switch (m_fit_mode){
    case WallpaperImageFitMode::FILL:
    case WallpaperImageFitMode::FIT:{
        QSizeF image_size = m_pixmap.deviceIndependentSize();
        QSizeF window_size = size();
        const qreal scale_x = window_size.width() / image_size.width();
        const qreal scale_y = window_size.height() / image_size.height();
        const qreal scale = m_fit_mode == WallpaperImageFitMode::FILL
            ? std::max(scale_x, scale_y)
            : std::min(scale_x, scale_y);
        QSizeF scaled_size = image_size * scale;
        QRectF destination(
            (window_size.width() - scaled_size.width()) / 2,
            (window_size.height() - scaled_size.height()) / 2,
            scaled_size.width(),
            scaled_size.height()
        );
        painter.drawPixmap(destination, m_pixmap, QRectF(m_pixmap.rect()));
        break;
    }
    case WallpaperImageFitMode::STRETCH:
        painter.drawPixmap(QRectF(rect()), m_pixmap, QRectF(m_pixmap.rect()));
        break;
    case WallpaperImageFitMode::TILE:
        painter.drawTiledPixmap(rect(), m_pixmap);
        break;
    }

    if (m_overlay > 0){
        int alpha = static_cast<int>(m_overlay) * 255 / 100;
        QColor overlay = m_surface_color;
        overlay.setAlpha(alpha);
        painter.fillRect(rect(), overlay);
    }
}

}
