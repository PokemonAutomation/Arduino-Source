/*  Global Settings Routes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtHttpServer/QHttpServerResponder>
#include <QJsonDocument>
#include <QJsonObject>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Server/HTTP.h"
#include "GlobalSettingsRoutes.h"

namespace PokemonAutomation{
namespace Server{

void register_settings_routes(){
    HTTPServer& server = HTTPServer::instance();

    // GET /settings
    server.addRoute(
        "/settings",
        QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest&,
        QHttpServerResponder& responder
    ){
        JsonValue json = GlobalSettings::instance().to_json();
        responder.write(QJsonDocument(to_QJson(json).toObject()));
    });

    // POST /settings
    server.addRoute(
        "/settings",
        QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest& request,
        QHttpServerResponder& responder
    ){
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (doc.isNull() || !doc.isObject()){
            responder.write(QHttpServerResponder::StatusCode::BadRequest);
            return;
        }

        GlobalSettings::instance().load_json(from_QJson(doc.object()));
        PERSISTENT_SETTINGS().write();

        responder.write(QHttpServerResponder::StatusCode::Ok);
    });
}

}
}
