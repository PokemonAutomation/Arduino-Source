/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include "CommonFramework/Globals.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "ProgramNotifications.h"

namespace PokemonAutomation{


void send_program_error_notification(
    Logger* logger,
    const QString& program,
    const QString& message
){
    QJsonArray embeds;
    {
        QJsonObject embed;
        {
            QString title = "Program Stopped (Error)";
            const QString& instance_name = PERSISTENT_SETTINGS().INSTANCE_NAME;
            if (!instance_name.isEmpty()){
                title += ": ";
                title += instance_name;
            }
            embed["title"] = title;
        }
        embed["color"] = 0xff0000;
        embed["author"] = QJsonObject();
        embed["image"] = QJsonObject();
        embed["thumbnail"] = QJsonObject();
        embed["footer"] = QJsonObject();

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = "Program (" + PROGRAM_VERSION + ")";
            field["value"] = program;
            fields.append(field);
        }
        {
            QJsonObject field;
            field["name"] = "Error Message";
            field["value"] = message;
            fields.append(field);
        }
        embed["fields"] = fields;
        embeds.append(embed);
    }
    DiscordWebHook::send_message(true, "", embeds, logger);
}


}
