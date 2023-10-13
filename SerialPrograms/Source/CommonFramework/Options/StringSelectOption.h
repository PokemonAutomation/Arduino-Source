/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringSelectOption_H
#define PokemonAutomation_Options_StringSelectOption_H

#include <vector>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{




struct StringSelectEntry{
    std::string slug;
    std::string display_name;
    ImageViewRGB32 icon;

    StringSelectEntry(std::string p_slug, std::string p_display_name)
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
    {}
    StringSelectEntry(std::string p_slug, std::string p_display_name, ImageViewRGB32 p_icon)
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
        , icon(std::move(p_icon))
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

public:
    const std::vector<StringSelectEntry>& case_list() const;

    const StringSelectEntry& operator[](size_t index) const;
    size_t search_index_by_slug(const std::string& slug) const;
    size_t search_index_by_name(const std::string& display_name) const;

private:
    struct Data;
    Pimpl<Data> m_data;
};



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
