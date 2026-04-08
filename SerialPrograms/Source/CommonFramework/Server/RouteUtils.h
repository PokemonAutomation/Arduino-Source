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
    // Normalise to NFD (Decomposition) to remove accents (é to e, etc...)
    slug = slug.normalized(QString::NormalizationForm_D);
    slug.remove(QRegularExpression("\\p{M}"));

    // Back to NFC for any further processing if needed, though not strictly required
    slug = slug.normalized(QString::NormalizationForm_C);

    // Reformat non-alphanumeric characters so "Program (1.0.0)" -> "program-1-0-0"
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
