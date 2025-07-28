/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_StringSelectOption_H
#define PokemonAutomation_CommonTools_Options_StringSelectOption_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{

class JsonValue;



struct StringSelectEntry{
    std::string slug;
    std::string display_name;
    ImageViewRGB32 icon;
    Color text_color;

    StringSelectEntry(
        std::string p_slug,
        std::string p_display_name,
        ImageViewRGB32 p_icon = ImageViewRGB32(),
        Color p_text_color = Color()
    )
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
        , icon(std::move(p_icon))
        , text_color(p_text_color)
    {}
};



class StringSelectDatabase{
public:
    ~StringSelectDatabase();
    StringSelectDatabase(StringSelectDatabase&&);
    StringSelectDatabase& operator=(StringSelectDatabase&&);
    StringSelectDatabase(const StringSelectDatabase&);
    StringSelectDatabase& operator=(const StringSelectDatabase&);

public:
    StringSelectDatabase();
    void add_entry(StringSelectEntry entry);
    size_t size() const;

public:
    const std::vector<StringSelectEntry>& case_list() const;
    size_t longest_text_length() const;

    const StringSelectEntry& operator[](size_t index) const;
    // if not found, return SIZE_MAX
    size_t search_index_by_slug(const std::string& slug) const;
    // if not found, return SIZE_MAX
    size_t search_index_by_name(const std::string& display_name) const;

private:
    struct Data;
    Pimpl<Data> m_data;
};

// Create a simple StringSelectDatabase from a list of slugs.
// The display names of each entry will be the same as their slugs.
StringSelectDatabase create_string_select_database(const std::vector<std::string>& slugs);
// Load a simple list of JSON strings to a StringSelectDatabase.
// Previous content of the StringSelectDatabase is removed if loading is successful.
// The display names of each loaded entry will be the same as their slugs.
// Return whether we successfully loaded from the JSON. If loading failed, the initial content
// of the database is not removed.
bool load_json_to_string_select_database(const JsonValue& json, StringSelectDatabase& database);

//  Config option that creates a cell where users can select a string from
//  its dropdown menu. It is best to put this cell in a table widget.
class StringSelectCell : public ConfigOption{
public:
    ~StringSelectCell();
    StringSelectCell(const StringSelectCell&) = delete;
    void operator=(const StringSelectCell&) = delete;

public:
    StringSelectCell(
        const StringSelectDatabase& database,
        LockMode lock_while_running,
        size_t default_index
    );
    StringSelectCell(
        const StringSelectDatabase& database,
        LockMode lock_while_running,
        const std::string& default_slug
    );

    size_t default_index() const;
    const std::string& default_slug() const;

    size_t index() const;
    const std::string& slug() const{
        return entry().slug;
    }
    const std::string& display_name() const{
        return entry().display_name;
    }
    const StringSelectEntry& entry() const{
        return database()[index()];
    }

    void set_by_index(size_t index);
    std::string set_by_slug(const std::string& slug);
    std::string set_by_name(const std::string& display_name);

    const StringSelectDatabase& database() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};


//  Config option that creates a dropdown menu for users to select
//  a sring. Different from StringSelectCell which is typically used 
//  in a table, StringSelectOption is considered a standalone option
//  that comes with its own label.
class StringSelectOption : public StringSelectCell{
public:
    StringSelectOption(
        std::string label,
        const StringSelectDatabase& database,
        LockMode lock_while_running,
        size_t default_index
    )
        : StringSelectCell(database, lock_while_running, default_index)
        , m_label(std::move(label))
    {}
    StringSelectOption(
        std::string label,
        const StringSelectDatabase& database,
        LockMode lock_while_running,
        const std::string& default_slug
    )
        : StringSelectCell(database, lock_while_running, default_slug)
        , m_label(std::move(label))
    {}

    const std::string& label() const{ return m_label; }
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    std::string m_label;
};





}
#endif
