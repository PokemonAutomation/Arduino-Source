/*  UI Wrapper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UiWrapper_H
#define PokemonAutomation_UiWrapper_H

#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{



class UiComponent{
public:
    UiComponent(const UiComponent& x) = delete;
    void operator=(const UiComponent& x) = delete;

    UiComponent() = default;
    virtual ~UiComponent() = default;
};



//  Lightweight smart pointer to avoid pulling in <memory>.
class UiWrapper{
public:
    ~UiWrapper(){
        if (m_owns){
            delete m_component;
        }
    }
    UiWrapper(UiWrapper&& x)
        : m_owns(x.m_owns)
        , m_component(x.m_component)
    {
        x.m_component = nullptr;
    }
    void operator=(UiWrapper&& x){
        if (this == &x){
            return;
        }
        if (m_owns){
            delete m_component;
        }
        m_owns = x.m_owns;
        m_component = x.m_component;
        x.m_component = nullptr;
    }
    UiWrapper(const UiWrapper& x) = delete;
    void operator=(const UiWrapper& x) = delete;


public:
    UiWrapper()
        : m_owns(false)
        , m_component(nullptr)
    {}
    UiWrapper(bool take_ownership, UiComponent* component)
        : m_owns(take_ownership)
        , m_component(component)
    {}

    bool is_owning() const{
        return m_owns;
    }
    UiComponent* release(){
        m_owns = false;
        return m_component;
    }


public:
    operator bool() const{
        return m_component != nullptr;
    }

    const UiComponent& operator*() const{
        return *m_component;
    }
    UiComponent& operator*(){
        return *m_component;
    }

    const UiComponent* operator->() const{
        return m_component;
    }
    UiComponent* operator->(){
        return m_component;
    }

    const UiComponent* get() const{
        return m_component;
    }
    UiComponent* get(){
        return m_component;
    }


private:
    bool m_owns;
    UiComponent* m_component;
};





template <typename UiStateType>
using UiFactory = UiWrapper (*)(UiStateType& state, void* params);



//
//  This is a convenience class that implementations should inherit from instead
//  of directly inheriting from ConfigOption or another option type.
//
//  This provides the per-type UI factory as well as the "make_ui_component"
//  override. This saves a ton of copy-paste as those are the same eveywhere.
//

template <typename... Args>
class UiState;


template <>
class UiState<>{
public:
    virtual UiWrapper make_ui_component(void* params) = 0;
};


template <typename Type>
class UiState<Type>{
public:
    virtual UiWrapper make_ui_component(void* params){
        if (m_ui_factory){
            return m_ui_factory(static_cast<Type&>(*this), params);
        }
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            std::string("UI component not registered for type: ") + typeid(Type).name()
        );
    }

    static UiFactory<Type> m_ui_factory;
};

template <typename Type>
UiFactory<Type> UiState<Type>::m_ui_factory;



template <typename Type, typename ParentType>
class UiState<Type, ParentType> : public ParentType{
public:
    using ParentType::ParentType;

    virtual UiWrapper make_ui_component(void* params) override{
        if (m_ui_factory){
            return m_ui_factory(static_cast<Type&>(*this), params);
        }
        return UiWrapper();
    }

    static UiFactory<Type> m_ui_factory;
};

template <typename Type, typename ParentType>
UiFactory<Type> UiState<Type, ParentType>::m_ui_factory;



}
#endif
