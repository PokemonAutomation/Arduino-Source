/*  Simple (and incomplete) Expression Evaluator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits>
#include <string>
#include <map>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "ExpressionEvaluator.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


bool is_whitespace(char ch){
    switch (ch){
    case ' ': return true;
    case '\t': return true;
    case '\r': return true;
    case '\n': return true;
    case '\0': return true;
    }
    return false;
}
bool is_digit(char ch){
    return '0' <= ch && ch <= '9';
}
bool is_symbol(char ch){
    if ('a' <= ch && ch <= 'z'){
        return true;
    }
    if ('A' <= ch && ch <= 'Z'){
        return true;
    }
    if (ch == '_'){
        return true;
    }
    return false;
}
bool skip_whitespace(const char*& str){
    while (true){
        char ch = *str;
        if (ch == '\0'){
            return true;
        }
        if (!is_whitespace(ch)){
            return false;
        }
        str++;
    }
}

template <typename T>
T checked_multiply(T x, T y){
    if (x == 0 || y == 0){
        return 0;
    }
    T ret = x * y;
    if (ret / x != y){
        throw ParseException("Overflow");
    }
    return ret;
}
template <typename T>
T checked_add(T x, T y){
    if constexpr (std::is_unsigned_v<T>){
        T ret = x + y;
        if (ret < x){
            throw ParseException("Overflow");
        }
        return ret;
    }else{
        if (x >= 0 && std::numeric_limits<T>::max() - x < y){
            throw ParseException("Overflow");
        }
        if (x < 0 && y < std::numeric_limits<T>::min() - x){
            throw ParseException("Overflow");
        }
        return x + y;
    }
}
template <typename T>
T checked_sub(T x, T y){
    if (x >= 0 && x - std::numeric_limits<T>::max() > y){
        throw ParseException("Overflow");
    }
    if (x < 0 && y > x - std::numeric_limits<T>::min()){
        throw ParseException("Overflow");
    }
    return x - y;
}

template <typename T>
T parse_integer(const char*& str){
    const char* ptr = str;

    T x = 0;
    while (true){
        char ch = *str;
        if (is_whitespace(ch)){
            return x;
        }
        if (is_digit(ch)){
            x = checked_multiply<T>(x, 10);
            x = checked_add<T>(x, ch - '0');
            str++;
            continue;
        }
        switch (ch){
        case '+':
        case '-':
        case '*':
        case ')':
            return x;
        }
        if (is_symbol(ch)){
            return x;
        }
        throw ParseException(std::string("Invalid integer: ") + ptr);
    }
}
std::string parse_symbol(const char*& str){
    const char* ptr = str;

    std::string ret;
    ret += *str++;
    while (true){
        char ch = *str;
        if (is_whitespace(ch)){
            return ret;
        }
        if (is_symbol(ch)){
            ret += ch;
            str++;
            continue;
        }
        if (is_digit(ch)){
            ret += ch;
            str++;
            continue;
        }
        switch (ch){
        case '+':
        case '-':
        case '*':
        case ')':
            return ret;
        }
        throw ParseException(std::string("Invalid symbol: ") + ptr);
    }
}

uint8_t precedence(char ch){
    switch (ch){
    case '+':
    case '-':
        return 0;
    case '*':
        return 1;
    }
    throw ParseException(std::string("Invalid operator: ") + ch);
}

int64_t parse_expression(
    const std::map<std::string, int64_t>& variables,
    const std::string& expression
){
    const char* str = expression.c_str();

    std::vector<std::pair<char, uint64_t>> num;
    std::vector<char> op;

    size_t consecutive_values = 0;
    while (true){
        //  Consecutive values is an implied multiply.
        if (consecutive_values > 1){
            consecutive_values = 0;
            op.push_back('*');
            while (op.size() >= 2){
                char top = op.back();
                char next = op[op.size() - 2];
                uint8_t p_top  = precedence(top);
                uint8_t p_next = precedence(next);
                if (p_top > p_next){
                    break;
                }
                op.pop_back();
                op.pop_back();
                num.emplace_back(next, 0);
                op.push_back(top);
                if (p_top == p_next){
                    break;
                }
            }
            continue;
        }

        if (skip_whitespace(str)){
            break;
        }
        char ch = *str;
//        cout << "ch = " << ch << endl;


        if (is_digit(ch)){
            consecutive_values++;
            num.emplace_back('\0', parse_integer<uint64_t>(str));
            continue;
        }
        if (is_symbol(ch)){
            consecutive_values++;
            std::string symbol = parse_symbol(str);
            auto iter = variables.find(symbol);
            if (iter == variables.end()){
                throw ParseException("Undefined Symbol: " + symbol);
            }
            num.emplace_back('\0', iter->second);
            continue;
        }
        if (ch == '+' || ch == '-' || ch == '*'){
            consecutive_values = 0;
            op.push_back(ch);
            str++;
            while (op.size() >= 2){
                char top = op.back();
                char next = op[op.size() - 2];
                uint8_t p_top  = precedence(top);
                uint8_t p_next = precedence(next);
                if (p_top > p_next){
                    break;
                }
                op.pop_back();
                op.pop_back();
                num.emplace_back(next, 0);
                op.push_back(top);
                if (p_top == p_next){
                    break;
                }
            }
            continue;
        }
    //    if (ch == '('){
    //
    //    }
    //    if (ch == ')'){
    //
    //    }
        throw ParseException("Invalid expression: " + expression);
    }

    while (!op.empty()){
        num.emplace_back(op.back(), 0);
        op.pop_back();
    }

#if 0
    cout << "operands: ";
    for (const auto& item : num){
        if (item.first == 0){
            cout << item.second << " ";
        }else{
            cout << item.first << " ";
        }
    }
    cout << endl;
    //cout << "operators: ";
    //for (const auto& item : op){
    //    cout << item << " ";
    //}
    //cout << endl;
#endif

    std::vector<int64_t> stack;
    for (const auto& item : num){
        if (item.first == 0){
            stack.push_back(item.second);
            num.pop_back();
            continue;
        }
        switch (item.first){
        case '+':{
            if (stack.size() < 2){
                throw ParseException("Invalid expression: unexpected +");
            }
            int64_t x = stack[stack.size() - 2];
            int64_t y = stack[stack.size() - 1];
            stack.pop_back();
            stack.pop_back();

            x = checked_add(x, y);
            stack.push_back(x);
            continue;
        }
        case '-':{
            if (stack.size() == 0){
                throw ParseException("Invalid expression: unexpected -");
            }

            // If we have at least two integers (x, y) in the stack, perform x - y
            // If we have only one integer y in the stack, perform - y (by leaving x = 0)

            int64_t y = stack[stack.size() - 1];
            stack.pop_back();

            int64_t x = 0;
            if (stack.size() > 0){
                x = stack[stack.size() - 1];
                stack.pop_back();
            }
            x = checked_sub(x, y);
            stack.push_back(x);
            continue;
        }
        case '*':{
            if (stack.size() < 2){
                throw ParseException("Invalid expression: unexpected *");
            }
            int64_t x = stack[stack.size() - 2];
            int64_t y = stack[stack.size() - 1];
            stack.pop_back();
            stack.pop_back();
            x = checked_multiply(x, y);
            stack.push_back(x);
            continue;
        }
        }
        throw ParseException("Invalid operator.");
    }
    if (stack.size() != 1){
        throw ParseException("Invalid expression.");
    }

    return stack[0];
}

const std::map<std::string, int64_t>& SYMBOLS(){
    static const std::map<std::string, int64_t> SYMBOLS{
        {"TICKS_PER_SECOND", 125},
    };
    return SYMBOLS;
}


uint32_t parse_ticks_ui32(const std::string& expression){
    int64_t x = parse_expression(SYMBOLS(), expression);
    if (x < 0){
        throw ParseException("Value cannot be negative.");
    }
    if ((int32_t)x != x){
        throw ParseException("Overflow");
    }
    return (int32_t)x;
}


int32_t parse_ticks_i32(const std::string& expression){
    int64_t x = parse_expression(SYMBOLS(), expression);
    if ((int32_t)x != x){
        throw ParseException("Overflow");
    }
    return (int32_t)x;
}


}


