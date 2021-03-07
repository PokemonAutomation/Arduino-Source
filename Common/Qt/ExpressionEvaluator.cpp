/*  Simple (and incomplete) Expression Evaluator
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <string>
#include <map>
#include <vector>
#include "StringException.h"
#include "ExpressionEvaluator.h"

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
uint32_t parse_integer(const char*& str){
    uint64_t x = 0;
    while (true){
        char ch = *str;
        if (is_whitespace(ch)){
            return (uint32_t)x;
        }
        if (is_digit(ch)){
            x *= 10;
            x += ch - '0';
            if ((uint32_t)x != x){
                throw StringException("Number is too large.");
            }
            str++;
            continue;
        }
        switch (ch){
        case '+':
        case '-':
        case '*':
        case ')':
            return (uint32_t)x;
        }
        throw StringException("Invalid integer.");
    }
}
std::string parse_symbol(const char*& str){
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
        throw StringException("Invalid symbol.");
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
    throw StringException("Invalid operator.");
}

int32_t parse_expression(
    const std::map<std::string, int64_t>& variables,
    const std::string& expression
){
    const char* str = expression.c_str();

    std::vector<std::pair<char, uint64_t>> num;
    std::vector<char> op;

    while (true){
        if (skip_whitespace(str)){
            break;
        }
        char ch = *str;
//        cout << "ch = " << ch << endl;
        if (is_digit(ch)){
            num.emplace_back(0, parse_integer(str));
            continue;
        }
        if (is_symbol(ch)){
            std::string symbol = parse_symbol(str);
            auto iter = variables.find(symbol);
            if (iter == variables.end()){
                throw StringException("Undefined symbol.");
            }
            num.emplace_back(0, iter->second);
            continue;
        }
        if (ch == '+' || ch == '-' || ch == '*'){
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
        throw StringException("Invalid expression.");
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
                throw StringException("Invalid expression: unexpected +");
            }
            int64_t x = stack[stack.size() - 2];
            int64_t y = stack[stack.size() - 1];
            stack.pop_back();
            stack.pop_back();
            x += y;
            if ((int32_t)x != x){
                throw StringException("Overflow");
            }
            stack.push_back(x);
            continue;
        }
        case '-':{
            if (stack.size() < 2){
                throw StringException("Invalid expression: unexpected -");
            }
            int64_t x = stack[stack.size() - 2];
            int64_t y = stack[stack.size() - 1];
            stack.pop_back();
            stack.pop_back();
            x -= y;
            if ((int32_t)x != x){
                throw StringException("Overflow");
            }
            stack.push_back(x);
            continue;
        }
        case '*':{
            if (stack.size() < 2){
                throw StringException("Invalid expression: unexpected *");
            }
            int64_t x = stack[stack.size() - 2];
            int64_t y = stack[stack.size() - 1];
            stack.pop_back();
            stack.pop_back();
            x *= y;
            if ((int32_t)x != x){
                throw StringException("Overflow");
            }
            stack.push_back(x);
            continue;
        }
        }
        throw StringException("Invalid operator.");
    }
    if (stack.size() != 1){
        throw StringException("Invalid expression.");
    }

    return (int32_t)stack[0];
}

const std::map<std::string, int64_t>& SYMBOLS(){
    static const std::map<std::string, int64_t> SYMBOLS{
        {"TICKS_PER_SECOND", 125},
    };
    return SYMBOLS;
}


uint32_t parse_ticks_i32(const QString& expression){
    int32_t x = parse_expression(SYMBOLS(), expression.toUtf8().toStdString());
    if (x < 0){
        throw StringException("Value cannot be negative.");
    }
    return x;
}



}


