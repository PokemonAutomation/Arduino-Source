/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringSelectOption_H
#define PokemonAutomation_Options_StringSelectOption_H

#include <vector>
#include <map>
#include <QIcon>
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{


class StringSelectOption : public ConfigOption{
public:
    StringSelectOption(
        std::string label,
        const std::vector<std::string>& cases,
        const std::string& default_case
    );
    StringSelectOption(
        std::string label,
        std::vector<std::pair<std::string, QIcon>> cases,
        const std::string& default_case
    );

    explicit operator size_t() const{ return m_current.load(std::memory_order_relaxed); }
    explicit operator const std::string&() const{ return m_case_list[(size_t)*this].first; }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class StringSelectWidget;
    const std::string m_label;
    std::vector<std::pair<std::string, QIcon>> m_case_list;
    std::map<std::string, size_t> m_case_map;
    size_t m_default;

    std::atomic<size_t> m_current;
};





struct StringSelectEntry{
    std::string slug;
    std::string display_name;
    QIcon icon;

    StringSelectEntry(std::string p_slug, std::string p_display_name)
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
    {}
    StringSelectEntry(std::string p_slug, std::string p_display_name, QIcon p_icon)
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
        , icon(std::move(p_icon))
    {}
};



class StringSelectDatabase{
public:
    const std::vector<StringSelectEntry>& case_list() const{ return m_list; }

    const StringSelectEntry& operator[](size_t index) const{ return m_list[index]; }
    size_t search_index_by_slug(const std::string& slug) const;
    size_t search_index_by_name(const std::string& display_name) const;

public:
    void add_entry(StringSelectEntry entry);

private:
    std::vector<StringSelectEntry> m_list;
    std::map<std::string, size_t> m_slug_to_entry;
    std::map<std::string, size_t> m_display_name_to_entry;
};



class StringSelectCell : public ConfigOption{
public:
    StringSelectCell(const StringSelectDatabase& database, size_t default_index);
    StringSelectCell(const StringSelectDatabase& database, const std::string& default_slug);

    size_t index() const{
        return m_index.load(std::memory_order_relaxed);
    }
    const std::string& slug() const{
        return entry().slug;
    }
    const std::string& display_name() const{
        return entry().display_name;
    }
    const StringSelectEntry& entry() const{
        return m_database[index()];
    }

    std::string set_by_slug(const std::string& slug);
    std::string set_by_name(const std::string& display_name);

    const StringSelectDatabase& database() const{ return m_database; }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const StringSelectDatabase& m_database;
    const size_t m_default;
    std::atomic<size_t> m_index;
};



class StringSelectOption2 : public StringSelectCell{
public:
    StringSelectOption2(
        std::string label,
        const StringSelectDatabase& database,
        size_t default_index
    )
        : StringSelectCell(database, default_index)
        , m_label(std::move(label))
    {}
    StringSelectOption2(
        std::string label,
        const StringSelectDatabase& database,
        const std::string& default_slug
    )
        : StringSelectCell(database, default_slug)
        , m_label(std::move(label))
    {}

    const std::string& label() const{ return m_label; }
    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::string m_label;
};





}
#endif
