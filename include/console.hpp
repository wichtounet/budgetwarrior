//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include "money.hpp"
#include "date.hpp"
#include "data_cache.hpp"

namespace budget {

inline std::string red(const std::string& str){
    return "\033[0;31m" + str + "\033[0;3047m";
}

inline std::string green(const std::string& str){
    return "\033[0;32m" + str + "\033[0;3047m";
}

inline std::string cyan(const std::string& str){
    return "\033[0;33m" + str + "\033[0;3047m";
}

std::string format_money(const budget::money& m);
std::string format_money_reverse(const budget::money& m);

inline std::string format_double(double m) {
    return format_money(money::from_double(m));
}

inline std::string format_double_reverse(double m) {
    return format_money_reverse(money::from_double(m));
}

/**
 * Returns the real size of a string. By default, accented characteres are
 * represented by several chars and make the length of the string being bigger
 * than its displayable length. This functionr returns only a size of 1 for an
 * accented chars.
 * \param value The string we want the real length for.
 * \return The real length of the string.
 */
size_t rsize(const std::string& value);
size_t rsize_after(const std::string& value);

template<typename T>
void print_minimum(std::ostream& os, const T& value, size_t min_width){
    auto str = to_string(value);

    auto old_width = os.width();
    os.width(min_width + (str.size() - rsize(str)));
    os << str;
    os.width(old_width);
}

template<typename T>
void print_minimum(const T& value, size_t min_width){
    print_minimum(std::cout, value, min_width);
}

template<typename T>
void print_minimum_left(std::ostream& os, const T& value, size_t min_width){
    auto str = to_string(value);

    if(rsize(str) >= min_width){
        os << str;
    } else {
        os << std::string(min_width - rsize(str), ' ');
        os << str;
    }
}

template<typename T>
void print_minimum_left(const T& value, size_t min_width){
    print_minimum_left(std::cout, value, min_width);
}

/**
 * Indicate if the given option was present in the list. If present, the option
 * is removed from the list.
 * \param option The full option name with any - included
 * \param args The command line arguments
 * \return true if the option was present, false otherwise.
 */
bool option(std::string_view option, std::vector<std::string>& args);

/**
 * Return the value of the given option if present or the default value
 * otherwise
 * \param option The full option name with any - included
 * \param args The command line arguments
 * \return The string value of the option or the default value is not present.
 */
std::string option_value(std::string_view option, std::vector<std::string>& args, const std::string& value);

std::string format_code(int attr, int fg, int bg);
std::string format_reset();

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

std::string get_string_complete(const std::vector<std::string>& choices);

template<typename ...Checker>
void edit_string_complete(std::string& ref, std::string_view title, const std::vector<std::string>& choices, Checker... checkers){
    bool checked = false;
    do {
        std::cout << title << " [" << ref << "]: ";

        auto answer = get_string_complete(choices);

        if(!answer.empty()){
            ref = answer;
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_string(std::string& ref, std::string_view title, Checker... checkers){
    bool checked = false;
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
void edit_number(size_t& ref, std::string_view title, Checker... checkers){
    bool checked = false;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if(!answer.empty()){
            ref = to_number<size_t>(answer);
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_number(int64_t& ref, std::string_view title, Checker... checkers){
    bool checked = false;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if (!answer.empty()) {
            ref = to_number<int64_t>(answer);
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_double(double& ref, std::string_view title, Checker... checkers){
    bool checked = false;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if(!answer.empty()){
            ref = to_number<double>(answer);
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_money(budget::money& ref, std::string_view title, Checker... checkers){
    bool checked = false;
    do {
        std::string answer;

        std::cout << title << " [" << ref << "]: ";
        std::getline(std::cin, answer);

        if(!answer.empty()){
            ref = money_from_string(answer);
        }

        checked = check(ref, checkers...);
    } while(!checked);
}

template<typename ...Checker>
void edit_date(date& ref, std::string_view title, Checker... checkers){
    bool checked = false;
    do {
        try {
            std::string answer;

            std::cout << title << " [" << ref << "]: ";
            std::getline(std::cin, answer);

            if(!answer.empty()){
                bool math = false;
                if(answer[0] == '+'){
                    std::string const str(std::next(answer.begin()), std::prev(answer.end()));
                    if(answer.back() == 'd'){
                        ref += days(static_cast<date_type>(std::stoul(str)));
                        math = true;
                    } else if(answer.back() == 'm'){
                        ref += months(static_cast<date_type>(std::stoul(str)));
                        math = true;
                    } else if(answer.back() == 'y'){
                        ref += years(static_cast<date_type>(std::stoul(str)));
                        math = true;
                    }
                } else if(answer[0] == '-'){
                    std::string const str(std::next(answer.begin()), std::prev(answer.end()));
                    if(answer.back() == 'd'){
                        ref -= days(static_cast<date_type>(std::stoul(str)));
                        math = true;
                    } else if(answer.back() == 'm'){
                        ref -= months(static_cast<date_type>(std::stoul(str)));
                        math = true;
                    } else if(answer.back() == 'y'){
                        ref -= years(static_cast<date_type>(std::stoul(str)));
                        math = true;
                    }
                }

                if(!math) {
                    ref = date_from_string(answer);
                }
            }

            checked = check(ref, checkers...);
        } catch(const date_exception& e){
            std::cout << e.message() << std::endl;
            checked = false;
        }

    } while(!checked);
}

struct not_empty_checker {
    bool operator()(const std::string& value) const {
        return !value.empty();
    }

    std::string message() const {
        return "This value cannot be empty";
    }
};

struct not_negative_checker {
    bool operator()(const budget::money& amount) const {
        return !(amount.dollars() < 0 || amount.cents() < 0);
    }

    std::string message() const {
        return "The amount cannot be negative";
    }
};

struct not_zero_checker {
    bool operator()(const budget::money& amount) const {
        return amount.dollars() > 0 || amount.cents() > 0;
    }

    std::string message() const {
        return "The amount cannot be negative";
    }
};

struct account_checker {
    budget::date date;

    account_checker() : date(budget::local_day()) {}
    explicit account_checker(budget::date date) : date(date) {}

    bool operator()(const std::string& value) const {
        data_cache cache;

        return !std::ranges::empty(all_accounts(cache, date.year(), date.month()) | filter_by_name(value));
    }

    std::string message() const {
        return "The account does not exist";
    }
};

struct asset_checker {
    bool operator()(const std::string& value) const {
        return asset_exists(value);
    }

    std::string message() const {
        return "The asset does not exist";
    }
};

struct liability_checker {
    bool operator()(const std::string& value) const {
        return liability_exists(value);
    }

    std::string message() const {
        return "The liability does not exist";
    }
};

struct share_asset_checker {
    bool operator()(const std::string& value) const {
        return share_asset_exists(value);
    }

    std::string message() const {
        return "The asset does not exist or is not share based";
    }
};

template<size_t First, size_t Last>
struct range_checker {
    bool operator()(const size_t& value) const {
        return value >= First && value <= Last;
    }

    std::string message() const {
        std::string m = "Value must in the range [";
        m += to_string(First);
        m += ", ";
        m += to_string(Last);
        m += "]";
        return m;
    }
};

struct one_of_checker {
    std::vector<std::string> values;
    explicit one_of_checker(std::vector<std::string> values) : values(std::move(std::move(values))) {
        //Nothing to init
    }

    bool operator()(const std::string& value) const {
        return std::ranges::find(values, value) != std::ranges::begin(values);
    }

    std::string message() const {
        std::string value = "This value can only be one of these values [";
        std::string comma;

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
