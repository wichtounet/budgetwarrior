//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
#include "recurring.hpp"
#include "report.hpp"
#include "fortune.hpp"
#include "objectives.hpp"
#include "wishes.hpp"
#include "versioning.hpp"
#include "version.hpp"

using namespace budget;

namespace {

typedef boost::mpl::vector<
            budget::debt_module*,
            budget::expenses_module*,
            budget::overview_module*,
            budget::accounts_module*,
            budget::earnings_module*,
            budget::recurring_module*,
            budget::fortune_module*,
            budget::report_module*,
            budget::objectives_module*,
            budget::wishes_module*,
            budget::versioning_module*,
            budget::version_module*,
            budget::help_module*
    > modules;

template<class T>
struct Void {
    typedef void type;
};

#define HAS_MEM_FUNC(func, name)                                        \
    template<typename T, typename Sign>                                 \
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename U, U> struct type_check;                     \
        template <typename _1> static yes &chk(type_check<Sign, &_1::func> *);    \
        template <typename   > static no  &chk(...);                              \
        static bool constexpr const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }

#define HAS_STATIC_FIELD(field, name)                                    \
template <class T>                                                       \
class name {                                                             \
    template<typename U, typename =                                      \
        typename std::enable_if<!std::is_member_pointer<decltype(&U::field)>::value>::type> \
    static std::true_type check(int);                                    \
    template <typename>                                                  \
    static std::false_type check(...);                                   \
public:                                                                  \
    static constexpr const bool value = decltype(check<T>(0))::value;    \
};

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

HAS_STATIC_FIELD(disable_preloading, has_disable_preloading_field)

template<typename Module, typename Enable = void>
struct disable_preloading {
    static const bool value = false;
};

template<typename Module>
struct disable_preloading<Module, typename std::enable_if<has_disable_preloading_field<module_traits<Module>>::value>::type> {
    static const bool value = module_traits<Module>::disable_preloading;
};

HAS_STATIC_FIELD(aliases, has_aliases_field)

template<typename Module, typename Enable = void>
struct has_aliases {
    static const bool value = false;
};

template<typename Module>
struct has_aliases<Module, typename std::enable_if<has_aliases_field<module_traits<Module>>::value>::type> {
    static const bool value = true;
};

template<bool B, typename T = void>
using disable_if = std::enable_if<!B, T>;

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
        //Preload each module that needs it
        if(!disable_preloading<Module>::value){
            module_loader loader;
            boost::mpl::for_each<modules>(boost::ref(loader));
        }

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

struct aliases_collector {
    std::vector<std::pair<const char*, const char*>> aliases;

    template<typename Module>
    inline typename std::enable_if<has_aliases<Module>::value, void>::type operator()(Module*){
        for(auto& v : module_traits<Module>::aliases){
            aliases.push_back(v);
        }
    }

    template<typename Module>
    inline typename disable_if<has_aliases<Module>::value, void>::type operator()(Module*){
        //NOP
    }
};

std::string exec_command(const std::string& command) {
    std::stringstream output;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        output << buffer;
    }

    pclose(stream);

    return output.str();
}

bool has_enough_colors(){
    auto colors = exec_command("tput colors");
    colors = colors.substr(0, colors.length() - 1);

    return to_number<int>(colors) > 4;
}

} //end of anonymous namespace

int main(int argc, const char* argv[]) {
    std::locale global_locale("");
    std::locale::global(global_locale);

    if(!load_config()){
        return 0;
    }

    if(!has_enough_colors()){
        std::cout << "WARNING: The terminal does not seem to have enough colors, some command may not work as intended" << std::endl;
    }

    auto old_data_version = to_number<std::size_t>(internal_config_value("data_version"));

    if(old_data_version > DATA_VERSION){
        std::cout << "Unsupported database version, you should update budgetwarrior" << std::endl;

        return 0;
    }

    if(old_data_version < DATA_VERSION){
        std::cout << "Migrate data base..." << std::endl;

        if(old_data_version == 1 && DATA_VERSION >= 2){
            migrate_recurring_1_to_2();
        }

        if(old_data_version <= 2 && DATA_VERSION >= 3){
            migrate_wishes_2_to_3();
        }

        internal_config_value("data_version") = to_string(DATA_VERSION);

        std::cout << "done" << std::endl;
    }
    
    //Collect all aliases
    aliases_collector collector;
    boost::mpl::for_each<modules>(boost::ref(collector));

    //Parse the command line args
    auto args = parse_args(argc, argv, collector.aliases);

    int code = 0;

    try {
        //Run the correct module
        module_runner runner(std::move(args));
        boost::mpl::for_each<modules>(boost::ref(runner));

        if(!runner.handled){
            std::cout << "Unhandled command \"" << runner.args[0] << "\"" << std::endl;

            code = 1;
        }
    } catch (const budget_exception& exception){
        std::cout << exception.message() << std::endl;

        code = 2;
    }

    save_config();

    return code;
}
