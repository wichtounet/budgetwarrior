//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "config.hpp"
#include "args.hpp"
#include "budget_exception.hpp"

//The different modules
#include "debts.hpp"
#include "accounts.hpp"
#include "expenses.hpp"
#include "overview.hpp"
#include "help.hpp"

using namespace budget;

namespace {

typedef boost::mpl::vector<
            budget::debt_module*,
            budget::expenses_module*,
            budget::overview_module*,
            budget::accounts_module*,
            budget::help_module*
    > modules;

struct module_runner {
    std::vector<std::string> args;
    bool handled = false;

    module_runner(std::vector<std::string>&& args) : args(std::forward<std::vector<std::string>>(args)) {
        //Nothing to init
    }

    template<typename Module>
    inline void handle_module(){
        Module module;

        module.handle(args);

        handled = true;
    }

    template<typename Module>
    inline void operator()(Module*){
        if(handled){
            return;
        }

        if(args.empty()){
            if(module_traits<Module>::is_default){
                handle_module<Module>();
            }
        } else if(args[0] == module_traits<Module>::command){
            handle_module<Module>();
        }
    }
};

} //end of anonymous namespace

int main(int argc, const char* argv[]) {
    std::locale global_locale("");
    std::locale::global(global_locale);

    if(!load_config()){
        return 0;
    }

    auto args = parse_args(argc, argv);

    try {
        module_runner runner(std::move(args));
        boost::mpl::for_each<modules>(boost::ref(runner));

        if(!runner.handled){
            std::cout << "Unhandled command \"" << runner.args[0] << "\"" << std::endl;

            return 1;
        }
    } catch (const budget_exception& exception){
        std::cout << exception.message() << std::endl;

        return 1;
    }

    return 0;
}
