/*  Discord WebHook
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordWebHook_H
#define PokemonAutomation_DiscordWebHook_H

#include "CommonFramework/Tools/Logger.h"

class QJsonArray;
class QJsonObject;
class QString;

namespace PokemonAutomation{
namespace DiscordWebHook{

void send_message(bool should_ping, const QString& message, const QJsonObject& embed, Logger* logger);
void send_message(bool should_ping, const QString& message, const QJsonArray& embeds, Logger* logger);

void send_message_old(bool should_ping, const QString& message, const QJsonArray& fields);

void send_file(QString file, Logger* logger);

}
}

#endif
