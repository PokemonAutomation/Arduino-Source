/*  Controller State Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "ControllerTypeStrings.h"
#include "ControllerStateTable.h"

namespace PokemonAutomation{



struct ControllerTypeEntry{
    ControllerCommandTable::RowFactory factory;
    std::vector<std::string> headers;
};

std::map<ControllerClass, ControllerTypeEntry>& controller_map(){
    static std::map<ControllerClass, ControllerTypeEntry> map;
    return map;
}



void ControllerCommandTable::register_controller_type(
    ControllerClass type,
    RowFactory factory,
    std::vector<std::string> headers
){
    auto& map = controller_map();
    if (map.emplace(
            type, ControllerTypeEntry{factory, std::move(headers)}
        ).second
    ){
        return;
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Duplicate Controller Enum: " + std::to_string((int)type)
    );
}


std::vector<std::string> ControllerCommandTable::make_header() const{
    auto& map = controller_map();
    auto iter = map.find(m_type);
    if (iter != map.end()){
        return iter->second.headers;
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Unsupported Controller Type: " + CONTROLLER_CLASS_STRINGS().get_string(m_type)
    );
}
std::unique_ptr<EditableTableRow> ControllerCommandTable::make_row(){
    auto& map = controller_map();
    auto iter = map.find(m_type);
    if (iter != map.end()){
        return iter->second.factory(*this);
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Unsupported Controller Type: " + CONTROLLER_CLASS_STRINGS().get_string(m_type)
    );
}







ControllerCommandTables::~ControllerCommandTables(){
    m_type.remove_listener(*this);
}
ControllerCommandTables::ControllerCommandTables(const std::vector<ControllerClass>& controller_list)
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , m_type(
        "<b>Controller Type:</b>",
        make_database(controller_list),
        LockMode::LOCK_WHILE_RUNNING,
        ControllerClass::NintendoSwitch_ProController
    )
    , m_table(
        ControllerClass::NintendoSwitch_ProController,
        "",
        LockMode::UNLOCK_WHILE_RUNNING
    )
{
    PA_ADD_OPTION(m_type);
    PA_ADD_OPTION(m_table);
    m_type.add_listener(*this);
}


void ControllerCommandTables::on_config_value_changed(void* object){
    m_table.clear();
    m_table.m_type = m_type;
}


EnumDropdownDatabase<ControllerClass> ControllerCommandTables::make_database(
    const std::vector<ControllerClass>& controller_list
){
    auto& map = controller_map();
    EnumDropdownDatabase<ControllerClass> ret;
    for (ControllerClass type : controller_list){
        if (map.find(type) == map.end()){
            throw InternalProgramError(
                nullptr,
                PA_CURRENT_FUNCTION,
                "Unsupported Controller Type: " + CONTROLLER_CLASS_STRINGS().get_string(type)
            );
        }
        const std::string& str = CONTROLLER_CLASS_STRINGS().get_string(type);
        ret.add(type, str, str, true);
    }
    return ret;
}






}
