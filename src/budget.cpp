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
#include "earnings.hpp"
#include "help.hpp"

using namespace budget;

namespace {

typedef boost::mpl::vector<
            budget::debt_module*,
            budget::expenses_module*,
            budget::overview_module*,
            budget::accounts_module*,
            budget::earnings_module*,
            budget::help_module*
    > modules;

#define HAS_MEM_FUNC(func, name)                                        \
    template<typename T, typename Sign>                                 \
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename U, U> struct type_check;                     \
        template <typename _1> static yes &chk(type_check<Sign, &_1::func> *); \
        template <typename   > static no  &chk(...);                    \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }

HAS_MEM_FUNC(load, has_load);
HAS_MEM_FUNC(unload, has_unload);
HAS_MEM_FUNC(preload, has_preload);

template<typename Module>
struct need_loading {
    static const bool value = has_load<Module, void(Module::*)()>::value;
};

template<typename Module>
struct need_unloading {
    static const bool value = has_unload<Module, void(Module::*)()>::value;
};

template<typename Module>
struct need_preloading {
    static const bool value = has_preload<Module, void(Module::*)()>::value;
};

template<bool B, typename T = void>
using disable_if = std::enable_if<!B, T>;

struct module_runner {
    std::vector<std::string> args;
    bool handled = false;

    module_runner(std::vector<std::string>&& args) : args(std::forward<std::vector<std::string>>(args)) {
        //Nothing to init
    }

    template<typename Module>
    inline typename std::enable_if<need_loading<Module>::value, void>::type load(Module& module){
       module.load();
    }

    template<typename Module>
    inline typename disable_if<need_loading<Module>::value, void>::type load(Module&){
        //NOP
    }

    template<typename Module>
    inline typename std::enable_if<need_unloading<Module>::value, void>::type unload(Module& module){
       module.unload();
    }

    template<typename Module>
    inline typename disable_if<need_unloading<Module>::value, void>::type unload(Module&){
        //NOP
    }

    template<typename Module>
    inline void handle_module(){
        Module module;

        load(module);

        module.handle(args);

        unload(module);

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

struct module_loader {
    template<typename Module>
    inline typename std::enable_if<need_preloading<Module>::value, void>::type preload(){
        Module module;
        module.preload();
    }

    template<typename Module>
    inline typename disable_if<need_preloading<Module>::value, void>::type preload(){
        //NOP
    }

    template<typename Module>
    inline void operator()(Module*){
        preload<Module>();
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
        //Preload each module that needs it
        module_loader loader;
        boost::mpl::for_each<modules>(boost::ref(loader));

        //Run the correct module
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
