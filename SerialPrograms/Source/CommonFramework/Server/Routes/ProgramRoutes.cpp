/*  Program Routes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QRegularExpression>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "CommonFramework/Server/HTTP.h"
#include "CommonFramework/Panels/ProgramRegistry.h"
#include "CommonFramework/Panels/PanelDescriptor.h"
#include "CommonFramework/Panels/PanelInstance.h"
#include "CommonFramework/Server/RouteUtils.h"
#include "ProgramRoutes.h"

namespace PokemonAutomation{
namespace Server{

void register_program_routes(){
    HTTPServer& server = HTTPServer::instance();

    // GET /programs
    server.addRoute(
        "/programs",
        QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest&,
        QHttpServerResponder& responder
    ){
        QJsonArray json_categories;
        for (const auto& category : ProgramRegistry::instance().categories()){
            QJsonObject category_obj;
            category_obj["display_name"] = QString::fromStdString(category->name());
            category_obj["slug"] = to_slug(category->name());
            json_categories.append(category_obj);
        }
        responder.write(QJsonDocument(json_categories));
    });

    // GET /programs/{category_slug}
    server.addRoute(
        "/programs/<arg>",
        QHttpServerRequest::Method::Get,
        [](QString category_slug,
        const QHttpServerRequest&,
        QHttpServerResponder& responder
    ){
        const auto& all_categories = ProgramRegistry::instance().categories();
        
        QString decoded_category_slug = QUrl::fromPercentEncoding(category_slug.toUtf8());

        PanelListDescriptor* found_category = nullptr;
        for (const auto& category : all_categories){
            QString category_name = QString::fromStdString(category->name());
            QString category_to_slug = to_slug(category->name());
            
            if (category_name.compare(decoded_category_slug, Qt::CaseInsensitive) == 0 ||
                category_to_slug.compare(category_slug, Qt::CaseInsensitive) == 0 ||
                category_to_slug.compare(decoded_category_slug, Qt::CaseInsensitive) == 0) {
                found_category = category.get();
                break;
            }
        }

        if (!found_category){
            responder.write(QHttpServerResponder::StatusCode::NotFound);
            return;
        }

        QJsonArray json_programs;
        for (const auto& program : found_category->get_panels()){
            if (program.descriptor){
                QJsonObject program_obj;
                program_obj["slug"] = to_slug(program.descriptor->display_name());
                program_obj["display_name"] = QString::fromStdString(program.descriptor->display_name());
                program_obj["description"] = QString::fromStdString(program.descriptor->description());
                json_programs.append(program_obj);
            }
        }
        responder.write(QJsonDocument(json_programs));
    });

    // GET /programs/{category_slug}/{program_slug}
    server.addRoute(
    "/programs/<arg>/<arg>",
        QHttpServerRequest::Method::Get,
        [](QString category_slug,
        QString program_slug,
        const QHttpServerRequest&,
        QHttpServerResponder& responder
    ){
        const auto& all_categories = ProgramRegistry::instance().categories();
        
        QString decoded_category_slug = QUrl::fromPercentEncoding(category_slug.toUtf8());
        QString decoded_program_slug = QUrl::fromPercentEncoding(program_slug.toUtf8());

        PanelListDescriptor* found_category = nullptr;
        for (const auto& category : all_categories){
            QString category_name = QString::fromStdString(category->name());
            QString category_to_slug = to_slug(category->name());
            
            if (category_name.compare(decoded_category_slug, Qt::CaseInsensitive) == 0 ||
                category_to_slug.compare(category_slug, Qt::CaseInsensitive) == 0 ||
                category_to_slug.compare(decoded_category_slug, Qt::CaseInsensitive) == 0) {
                found_category = category.get();
                break;
            }
        }

        if (!found_category){
            responder.write(QHttpServerResponder::StatusCode::NotFound);
            return;
        }

        for (const auto& program : found_category->get_panels()){
            if (program.descriptor){
                QString program_name = QString::fromStdString(program.descriptor->display_name());
                QString program_to_slug = to_slug(program.descriptor->display_name());

                if (program_name.compare(decoded_program_slug, Qt::CaseInsensitive) == 0 ||
                    program_to_slug.compare(program_slug, Qt::CaseInsensitive) == 0 ||
                    program_to_slug.compare(decoded_program_slug, Qt::CaseInsensitive) == 0) {
                    
                    QJsonObject program_obj;
                    program_obj["slug"] = program_to_slug;
                    program_obj["display_name"] = program_name;
                    program_obj["description"] = QString::fromStdString(program.descriptor->description());
                    program_obj["category"] = QString::fromStdString(program.descriptor->category());
                    
                    responder.write(QJsonDocument(program_obj));
                    return;
                }
            }
        }

        responder.write(QHttpServerResponder::StatusCode::NotFound);
    });

    // GET /programs/{category_slug}/{program_slug}/options
    server.addRoute(
        "/programs/<arg>/<arg>/options",
        QHttpServerRequest::Method::Get,
        [](QString category_slug,
        QString program_slug,
        const QHttpServerRequest&,
        QHttpServerResponder& responder
    ){
        const auto& all_categories = ProgramRegistry::instance().categories();
        QString decoded_category_slug = QUrl::fromPercentEncoding(category_slug.toUtf8());
        QString decoded_program_slug = QUrl::fromPercentEncoding(program_slug.toUtf8());

        for (const auto& category : all_categories){
            if (QString::fromStdString(category->name()).compare(decoded_category_slug, Qt::CaseInsensitive) == 0 ||
                to_slug(category->name()).compare(category_slug, Qt::CaseInsensitive) == 0) {
                
                for (const auto& program : category->get_panels()){
                    if (program.descriptor && (QString::fromStdString(program.descriptor->display_name()).compare(decoded_program_slug, Qt::CaseInsensitive) == 0 ||
                                            to_slug(program.descriptor->display_name()).compare(program_slug, Qt::CaseInsensitive) == 0)) {
                        
                        std::unique_ptr<PanelInstance> instance = program.descriptor->make_panel();
                        // Load saved settings
                        instance->from_json();
                        QJsonValue qjson = to_QJson(instance->to_json());
                        responder.write(QJsonDocument(qjson.toObject()));
                        return;
                    }
                }
            }
        }
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    });

    // POST /programs/{category_slug}/{program_slug}/options
    server.addRoute(
        "/programs/<arg>/<arg>/options",
        QHttpServerRequest::Method::Post,
        [](QString category_slug,
        QString program_slug,
        const QHttpServerRequest& request,
        QHttpServerResponder& responder
    ){
        const auto& all_categories = ProgramRegistry::instance().categories();
        QString decoded_category_slug = QUrl::fromPercentEncoding(category_slug.toUtf8());
        QString decoded_program_slug = QUrl::fromPercentEncoding(program_slug.toUtf8());

        for (const auto& category : all_categories){
            if (QString::fromStdString(category->name()).compare(decoded_category_slug, Qt::CaseInsensitive) == 0 ||
                to_slug(category->name()).compare(category_slug, Qt::CaseInsensitive) == 0) {
                
                for (const auto& program : category->get_panels()){
                    if (program.descriptor && (QString::fromStdString(program.descriptor->display_name()).compare(decoded_program_slug, Qt::CaseInsensitive) == 0 ||
                                            to_slug(program.descriptor->display_name()).compare(program_slug, Qt::CaseInsensitive) == 0)) {
                        
                        QJsonDocument doc = QJsonDocument::fromJson(request.body());
                        if (doc.isNull() || !doc.isObject()){
                            responder.write(QHttpServerResponder::StatusCode::BadRequest);
                            return;
                        }

                        std::unique_ptr<PanelInstance> instance = program.descriptor->make_panel();
                        instance->from_json(); // Load first to preserve other options not in the POST body
                        instance->from_json(from_QJson(doc.object()));
                        instance->save_settings();

                        responder.write(QHttpServerResponder::StatusCode::Ok);
                        return;
                    }
                }
            }
        }
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    });
}

}
}
