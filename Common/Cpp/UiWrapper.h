/*  UI Wrapper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UiWrapper_H
#define PokemonAutomation_UiWrapper_H

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
    UiWrapper()
        : m_owns(false)
        , m_component(nullptr)
    {}
    UiWrapper(bool take_ownership, UiComponent* component)
        : m_owns(take_ownership)
        , m_component(component)
    {}
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




}
#endif
