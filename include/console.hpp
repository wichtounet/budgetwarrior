//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CONSOLE_H
#define CONSOLE_H

#include <vector>
#include <string>

#include "money.hpp"
#include "date.hpp"
#include "accounts.hpp"

namespace budget {

/**
 * Returns the real size of a string. By default, accented characteres are
 * represented by several chars and make the length of the string being bigger
 * than its displayable length. This functionr returns only a size of 1 for an
 * accented chars.
 * \param value The string we want the real length for.
 * \return The real length of the string.
 */
std::size_t rsize(const std::string& value);

template<typename T>
void print_minimum(const T& value, std::size_t min_width){
    auto str = to_string(value);

    auto old_width = std::cout.width();
    std::cout.width(min_width + (str.size() - rsize(str)));
    std::cout << str;
    std::cout.width(old_width);
}

/**
 * Indicate if the given option was present in the list. If present, the option
 * is removed from the list. 
 * \param option The full option name with any - included
 * \param args The command line arguments
 * \return true if the option was present, false otherwise.
 */
bool option(const std::string& option, std::vector<std::string>& args);

/**
 * Return the value of the given option if present or the default value
 * otherwise
 * \param option The full option name with any - included
 * \param args The command line arguments
 * \return The string value of the option or the default value is not present. 
 */
std::string option_value(const std::string& option, std::vector<std::string>& args, const std::string& value);

std::string format_code(int attr, int fg, int bg);
std::string format(const std::string& value);
void display_table(std::vector<std::string> columns, std::vector<std::vector<std::string>> contents, std::size_t groups = 1);

template<typename T>
bool check(const T&){
    return true;
}

template<typename T, typename CheckerA, typename ...Checker>
bool check(const T& value, CheckerA first, Checker... checkers){
    if(!first(value)){
        std::cout << first.message() << std::endl;
        return false;
    }

    return check(value, checkers...);
}

template<typename ...Checker>
void edit_string(std::string& ref, const std::string& title, Checker... checkers){
    bool checked;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if(!answer.empty()){
            ref = answer;
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_money(budget::money& ref, const std::string& title, Checker... checkers){
    bool checked;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if(!answer.empty()){
            ref = parse_money(answer);
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_date(date& ref, const std::string& title, Checker... checkers){
    bool checked;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if(!answer.empty()){
            bool math = false;
            if(answer[0] == '+'){
                std::string str(std::next(answer.begin()), std::prev(answer.end()));
                if(answer.back() == 'd'){
                    ref += boost::gregorian::date_duration(std::stoi(str));
                    math = true;
                } else if(answer.back() == 'm'){
                    ref += boost::gregorian::months(std::stoi(str));
                    math = true;
                } else if(answer.back() == 'y'){
                    ref += boost::gregorian::years(std::stoi(str));
                    math = true;
                }
            } else if(answer[0] == '-'){
                std::string str(std::next(answer.begin()), std::prev(answer.end()));
                if(answer.back() == 'd'){
                    ref -= boost::gregorian::date_duration(std::stoi(str));
                    math = true;
                } else if(answer.back() == 'm'){
                    ref -= boost::gregorian::months(std::stoi(str));
                    math = true;
                } else if(answer.back() == 'y'){
                    ref -= boost::gregorian::years(std::stoi(str));
                    math = true;
                }
            } 
            
            if(!math) {
                ref = boost::gregorian::from_string(answer);
            }
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

struct not_empty_checker {
    bool operator()(const std::string& value){
        return !value.empty();
    }

    std::string message(){
        return "This value cannot be empty";
    }
};

struct not_negative_checker {
    bool operator()(const budget::money& amount){
        return !(amount.dollars() < 0 || amount.cents() < 0);
    }

    std::string message(){
        return "The amount cannot be negative";
    }
};

struct not_zero_checker {
    bool operator()(const budget::money& amount){
        return amount.dollars() > 0 || amount.cents() > 0;
    }

    std::string message(){
        return "The amount cannot be negative";
    }
};

struct account_checker {
    bool operator()(const std::string& value){
        return account_exists(value);
    }

    std::string message(){
        return "The account does not exist";
    }
};

struct one_of_checker {
    std::vector<std::string> values; 
    one_of_checker(std::vector<std::string> values) : values(values){
        //Nothing to init
    }

    bool operator()(const std::string& value){
        for(auto& v : values){
            if(value == v){
                return true;
            }
        }

        return false;
    }

    std::string message(){
        std::string value = "This value can only be one of these values [";
        std::string comma = "";

        for(auto& v : values){
            value += comma;
            value += v;
            comma = ", ";
        }

        value += "]";

        return value;
    }
};

} //end of namespace budget

#endif
