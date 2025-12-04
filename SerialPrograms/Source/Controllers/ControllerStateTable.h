/*  Controller State Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerStateTable_H
#define PokemonAutomation_Controllers_ControllerStateTable_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "ControllerTypes.h"
#include "ControllerState.h"

namespace PokemonAutomation{




class ControllerStateRow : public EditableTableRow{
public:
    using EditableTableRow::EditableTableRow;
    virtual std::unique_ptr<ControllerState> get_state(Milliseconds& duration) const = 0;
};



class ControllerCommandTable : public EditableTableOption{
public:
    using RowFactory = std::unique_ptr<ControllerStateRow> (*)(ControllerCommandTable& parent);

    static void register_controller_type(
        ControllerClass type,
        RowFactory factory,
        std::vector<std::string> headers
    );

    template <typename RowType>
    static std::unique_ptr<ControllerStateRow> make_row(ControllerCommandTable& parent){
        return std::unique_ptr<ControllerStateRow>(new RowType(parent));
    }


public:
    ControllerCommandTable(
        ControllerClass type,
        std::string label,
        LockMode lock_while_running
    )
        : EditableTableOption(std::move(label), lock_while_running)
        , m_type(type)
    {}

    ControllerClass type() const{
        return m_type;
    }

    void run(
        CancellableScope& scope,
        AbstractController& controller
    ){
        std::vector<std::unique_ptr<ControllerStateRow>> table = EditableTableOption::copy_snapshot<ControllerStateRow>();
        for (std::unique_ptr<ControllerStateRow>& command : table){
            Milliseconds duration;
            std::unique_ptr<ControllerState> state = command->get_state(duration);
            state->execute(scope, controller, duration);
        }
    }

    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() override;


private:
    friend class ControllerCommandTables;
    ControllerClass m_type;
};





class ControllerCommandTables : public BatchOption, public ConfigOption::Listener{
public:
    ~ControllerCommandTables();
    ControllerCommandTables(const std::vector<ControllerClass>& controller_list);

public:
    virtual void on_config_value_changed(void* object) override;

private:
    static EnumDropdownDatabase<ControllerClass> make_database(
        const std::vector<ControllerClass>& controller_list
    );

private:
    //  There is a race condition here if multiple threads try to change this
    //  at the same time since it clears the table. But we only ever do this
    //  from the main Qt thread.
    EnumDropdownOption<ControllerClass> m_type;

    ControllerCommandTable m_table;
};







}
#endif
