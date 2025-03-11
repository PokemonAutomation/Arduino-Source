/*  DLL Safe String
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DllSafeString_H
#define PokemonAutomation_DllSafeString_H

#include <string.h>
#include <ostream>
#include <string>

namespace PokemonAutomation{


class DllSafeString{
public:
    //  Rule of 5
    ~DllSafeString(){ clear(); }
    DllSafeString(const DllSafeString&) = delete;
    void operator=(const DllSafeString&) = delete;
    DllSafeString(DllSafeString&& x)
        : m_ptr(x.m_ptr)
        , m_size(x.m_size)
        , m_deleter(x.m_deleter)
    {
        x.m_ptr = nullptr;
        x.m_size = 0;
        x.m_deleter = nullptr;
    }
    void operator=(DllSafeString&& x){
        clear();
        m_ptr = x.m_ptr;
        m_size = x.m_size;
        m_deleter = x.m_deleter;
        x.m_ptr = nullptr;
        x.m_size = 0;
        x.m_deleter = nullptr;
    }


public:
    DllSafeString()
        : m_ptr(nullptr)
        , m_size(0)
        , m_deleter(nullptr)
    {}
    DllSafeString(const char* str)
        : DllSafeString(std::string(str))
    {}
    DllSafeString(const std::string& str)
        : m_ptr(new char[str.size()])
        , m_size(str.size())
        , m_deleter(&deleter)
    {
        memcpy(m_ptr, str.c_str(), str.size());
    }
    void clear(){
        if (m_ptr != nullptr){
            m_deleter(m_ptr);
            m_size = 0;
            m_deleter = nullptr;
        }
    }


public:
    operator std::string() const{
        return std::string(m_ptr, m_size);
    }
    const char* c_str() const{
        return m_ptr;
    }
    bool empty() const{
        return m_size == 0;
    }
    size_t size() const{
        return m_size;
    }

    char& operator[](size_t index){
        return m_ptr[index];
    }
    char operator[](size_t index) const{
        return m_ptr[index];
    }

    friend std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const DllSafeString& str){
        return os << str.c_str();
    }


private:
    static void deleter(char* ptr){
        delete[] ptr;
    }


private:
    char* m_ptr;
    size_t m_size;
    void (*m_deleter)(char*);
};



}
#endif
