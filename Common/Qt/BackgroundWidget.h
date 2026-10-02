/*  Background Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_BackgroundWidget_H
#define PokemonAutomation_BackgroundWidget_H

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

struct BackgroundImageResult{
    QImage image;
    QString error;
};

class BackgroundWidget : public QWidget{
public:
    BackgroundWidget(QWidget* parent, std::function<void()> on_loaded);
    ~BackgroundWidget();

    void set_appearance(
        BackgroundImageFitMode fit_mode,
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
    QFutureWatcher<BackgroundImageResult>* m_loading = nullptr;
    uint64_t m_generation = 0;
    bool m_tile = false;
    bool m_failed = false;
    QString m_error;
    QString m_path;
    QPixmap m_pixmap;
    BackgroundImageFitMode m_fit_mode = BackgroundImageFitMode::FILL;
    uint8_t m_overlay = 35;
    QColor m_surface_color;
};

}

#endif
