/*  Route Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Server_RouteUtils_H
#define PokemonAutomation_Server_RouteUtils_H

#include <QString>
#include <QRegularExpression>
#include <string>

namespace PokemonAutomation{
namespace Server{

inline QString to_slug(const std::string& name) {
    QString slug = QString::fromStdString(name);
    // Normalize to NFD (Decomposition) to separate accents from base characters
    slug = slug.normalized(QString::NormalizationForm_D);
    // Remove Non-Spacing Mark characters (diacritics)
    slug.remove(QRegularExpression("\\p{M}"));
    // Back to NFC for any further processing if needed, though not strictly required for slugification
    slug = slug.normalized(QString::NormalizationForm_C);
    
    slug = slug.toLower();
    slug.replace(QRegularExpression("[^a-z0-9]"), "-");
    while (slug.contains("--")) {
        slug.replace("--", "-");
    }
    if (slug.startsWith("-")) slug.remove(0, 1);
    if (slug.endsWith("-")) slug.remove(slug.length() - 1, 1);
    return slug;
}

}
}

#endif
