/*  Sprite Composite Image
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Resources_SpriteCompositeImage_H
#define PokemonAutomation_CommonTools_Resources_SpriteCompositeImage_H

#include <map>
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{


class SpriteDatabase{
public:
    //  Build from a composite image.
    //
    //  The location of each sprite on the image is specified by a json file.
    //  Return a map of pokemon slug name for each sprite -> QIcon of the sprite.
    //
    //  The json file must have the following format:
    //  {
    //      "spriteWidth": <width of each sprite>,
    //      "spriteHeight": <height of each sprite>,
    //      "spriteLocations": {
    //          "<pokemon_slug_for sprite>" : {
    //              "top": <how many pixels from this sprite to the top edge of the image>,
    //              "left":  <how many pixels from the sprite to the left edge of the image>
    //          },
    //          (next pokemon) ...
    //      }
    //  }
    SpriteDatabase(const char* sprite_path, const char* json_path);

public:
    struct Sprite{
        ImageViewRGB32 sprite;  //  The original sprite.
        ImageViewRGB32 icon;    //  Sprite with 0-alpha boundaries cropped for better viewing.
    };
    const Sprite& get_throw(const std::string& slug) const;
    const Sprite* get_nothrow(const std::string& slug) const;

public:
    using const_iterator = std::map<std::string, Sprite>::const_iterator;
    using       iterator = std::map<std::string, Sprite>::iterator;

    const_iterator cbegin   () const{ return m_database.cbegin(); }
    const_iterator begin    () const{ return m_database.begin(); }
          iterator begin    (){ return m_database.begin(); }
    const_iterator cend   () const{ return m_database.cend(); }
    const_iterator end    () const{ return m_database.end(); }
          iterator end    (){ return m_database.end(); }

private:
    std::map<std::string, Sprite> m_database;
    ImageRGB32 m_backing_image;
};



}
#endif
