//=======================================================================
// Copyright (c) 2013-2018 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <sstream>

#include "cpp_utils/assert.hpp"
#include "cpp_utils/string.hpp"

#include "console.hpp"

// For getch
#include <termios.h>
#include <unistd.h>

std::string budget::format_code(int attr, int fg, int bg) {
    std::stringstream stream;
    stream << "" << '\033' << "[" << attr << ";" << (fg + 30) << (bg + 40) << "m";
    return stream.str();
}

std::string budget::format_reset() {
    return format_code(0, 0, 7);
}

std::string budget::format_money(const budget::money& m) {
    if (m.positive()) {
        return "::green" + budget::to_string(m);
    } else if (m.negative()) {
        return "::red" + budget::to_string(m);
    } else {
        return budget::to_string(m);
    }
}

std::string budget::format_money_no_color(const budget::money& m) {
    return budget::to_string(m);
}

std::string budget::format_money_reverse(const budget::money& m) {
    if (m.positive()) {
        return "::red" + budget::to_string(m);
    } else if (m.negative()) {
        return "::green" + budget::to_string(m);
    } else {
        return budget::to_string(m);
    }
}

size_t budget::rsize(const std::string& value) {
    auto v = value;

    if (v.substr(0, 5) == "::red") {
        v = v.substr(5);
    } else if (v.substr(0, 7) == "::green") {
        v = v.substr(7);
    }

    static wchar_t buf[1025];
    return mbstowcs(buf, v.c_str(), 1024);
}

size_t budget::rsize_after(const std::string& value) {
    auto v = value;

    auto index = v.find('\033');

    while (index != std::string::npos) {
        if (v[index + 3] == 'm') {
            v.erase(index, 4);
        } else if (v[index + 6] == 'm') {
            v.erase(index, 7);
        } else {
            v.erase(index, 9);
        }

        index = v.find('\033');
    }

    static wchar_t buf[1025];
    return mbstowcs(buf, v.c_str(), 1024);
}

bool budget::option(const std::string& option, std::vector<std::string>& args) {
    auto before = args.size();
    args.erase(std::remove(args.begin(), args.end(), option), args.end());
    return before != args.size();
}

std::string budget::option_value(const std::string& option, std::vector<std::string>& args, const std::string& default_value) {
    auto it  = args.begin();
    auto end = args.end();

    auto value = default_value;

    while (it != end) {
        if (it->find(option + "=") == 0) {
            value = std::string(it->begin() + option.size() + 1, it->end());

            it  = args.erase(it);
            end = args.end();
        } else {
            ++it;
        }
    }

    return value;
}

namespace {

char getch() {
    char buf = 0;
    struct termios old;
    fflush(stdout);

    if (tcgetattr(0, &old) < 0) {
        perror("tcsetattr()");
    }

    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN]  = 1;
    old.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }

    if (read(0, &buf, 1) < 0) {
        perror("read()");
    }

    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;

    if (tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }

    return buf;
}

} // end of anonymous namespace

std::string budget::get_string_complete(const std::vector<std::string>& choices) {
    std::string answer;

    if (choices.empty()) {
        std::getline(std::cin, answer);
        return answer;
    }

    size_t index = 0;

    while (true) {
        char c = getch();

        if (+c == 127) {
            if (!answer.empty()) {
                std::cout << "\b \b";
                answer.pop_back();
            }
        } else if (c == '\r') {
            std::cout << std::endl;
            return answer;
        } else if (c == '\n') {
            std::cout << std::endl;
            return answer;
        } else if (c == '\t') {
            if (!answer.empty()) {
                size_t count = 0;
                std::string valid;

                for (auto& choice : choices) {
                    if (choice.size() > answer.size()) {
                        if (choice.substr(0, answer.size()) == answer) {
                            ++count;
                            valid = choice;
                        }
                    }
                }

                if (count == 1) {
                    std::cout << valid.substr(answer.size(), valid.size());
                    answer = valid;
                }
            }
        } else if (c == '\033') {
            getch();

            char cc = getch();

            if (cc == 'A') {
                for (size_t i = 0; i < answer.size(); ++i) {
                    std::cout << "\b \b";
                }

                index = (index + 1) % (choices.size() + 1);

                if (index > 0) {
                    answer = choices[index - 1];
                } else if (index == 0) {
                    answer = "";
                }

                std::cout << answer;
            } else if (cc == 'B') {
                for (size_t i = 0; i < answer.size(); ++i) {
                    std::cout << "\b \b";
                }

                if (index == 1) {
                    index  = 0;
                    answer = "";
                } else if (index == 0) {
                    index  = choices.size();
                    answer = choices[index - 1];
                } else {
                    --index;
                    answer = choices[index - 1];
                }

                std::cout << answer;
            }
        } else {
            std::cout << c;
            answer += c;
        }
    }

    return answer;
}
