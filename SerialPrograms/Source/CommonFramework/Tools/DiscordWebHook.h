/*  Discord WebHook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordWebHook_H
#define PokemonAutomation_DiscordWebHook_H

#include <QImage>
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Tools/Logger.h"

class QJsonArray;
class QJsonObject;
class QString;

namespace PokemonAutomation{
namespace DiscordWebHook{


void send_message(Logger& logger, bool should_ping, const QString& message, const QJsonObject& embed);
void send_message(Logger& logger, bool should_ping, const QString& message, const QJsonArray& embeds);

void send_file(Logger& logger, QString file, bool keep_file);
void send_image(Logger& logger, const QImage& image, const QString& format, bool keep_file);
void send_screenshot(Logger& logger, const QImage& image, ScreenshotMode mode, bool keep_file);


}
}
#endif
