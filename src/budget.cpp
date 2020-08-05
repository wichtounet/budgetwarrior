//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <string>
#include <iostream>
#include <tuple>

#include "cpp_utils/tmp.hpp"

#include "config.hpp"
#include "args.hpp"
#include "budget_exception.hpp"
#include "api.hpp"
#include "currency.hpp"
#include "share.hpp"

//The different modules
#include "debts.hpp"
#include "accounts.hpp"
#include "incomes.hpp"
#include "assets.hpp"
#include "liabilities.hpp"
#include "expenses.hpp"
#include "overview.hpp"
#include "summary.hpp"
#include "earnings.hpp"
#include "help.hpp"
#include "recurring.hpp"
#include "report.hpp"
#include "fortune.hpp"
#include "objectives.hpp"
#include "wishes.hpp"
#include "versioning.hpp"
#include "version.hpp"
#include "predict.hpp"
#include "gc.hpp"
#include "server.hpp"
#include "retirement.hpp"

using namespace budget;

namespace {

typedef std::tuple<
            budget::server_module,
            budget::debt_module,
            budget::expenses_module,
            budget::overview_module,
            budget::summary_module,
            budget::accounts_module,
            budget::incomes_module,
            budget::assets_module,
            budget::liabilities_module,
            budget::earnings_module,
            budget::recurring_module,
            budget::fortune_module,
            budget::report_module,
            budget::objectives_module,
            budget::wishes_module,
            budget::versioning_module,
            budget::version_module,
            budget::predict_module,
            budget::retirement_module,
            budget::gc_module,
            budget::help_module
    > modules_tuple;

template<class T>
struct Void {
    typedef void type;
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
struct disable_preloading<Module, std::enable_if_t<has_disable_preloading_field<module_traits<Module>>::value>> {
    static const bool value = module_traits<Module>::disable_preloading;
};

HAS_STATIC_FIELD(aliases, has_aliases_field)

template<typename Module, typename Enable = void>
struct has_aliases {
    static const bool value = false;
};

template<typename Module>
struct has_aliases<Module, std::enable_if_t<has_aliases_field<module_traits<Module>>::value>> {
    static const bool value = true;
};

struct module_loader {
    template<typename Module, cpp::enable_if_u<need_preloading<Module>::value> = cpp::detail::dummy>
    inline void preload(){
        Module module;
        module.preload();
    }

    template<typename Module, cpp::disable_if_u<need_preloading<Module>::value> = cpp::detail::dummy>
    inline void preload(){
        //NOP
    }

    template<typename Module>
    inline void operator()(){
        preload<Module>();
    }
};

struct module_runner {
    std::vector<std::string> args;
    bool handled = false;

    module_runner(std::vector<std::string>&& args) : args(std::forward<std::vector<std::string>>(args)) {
        //Nothing to init
    }

    template<typename Module, cpp::enable_if_u<need_loading<Module>::value> = cpp::detail::dummy>
    inline void load(Module& module){
       module.load();
    }

    template<typename Module, cpp::disable_if_u<need_loading<Module>::value> = cpp::detail::dummy>
    inline void load(Module&){
        //NOP
    }

    template<typename Module, cpp::enable_if_u<need_unloading<Module>::value> = cpp::detail::dummy>
    inline void unload(Module& module){
       module.unload();
    }

    template<typename Module, cpp::disable_if_u<need_unloading<Module>::value> = cpp::detail::dummy>
    inline void unload(Module&){
        //NOP
    }

    template<typename Module>
    inline void handle_module(){
        //Preload each module that needs it
        if(!disable_preloading<Module>::value){
            module_loader loader;
            cpp::for_each_tuple_t<modules_tuple>(loader);
        }

        Module module;

        load(module);

        module.handle(args);

        unload(module);

        handled = true;
    }

    template<typename Module>
    inline void operator()(){
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

    template<typename Module, cpp::enable_if_u<has_aliases<Module>::value> = cpp::detail::dummy>
    inline void operator()(){
        for(auto& v : module_traits<Module>::aliases){
            aliases.push_back(v);
        }
    }

    template<typename Module, cpp::disable_if_u<has_aliases<Module>::value> = cpp::detail::dummy>
    inline void operator()(){
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

    //Collect all aliases
    aliases_collector collector;
    cpp::for_each_tuple_t<modules_tuple>(collector);

    //Parse the command line args
    auto args = parse_args(argc, argv, collector.aliases);

    if(args.size() && args[0] == "server"){
        set_server_running();
    }

    if (!load_config()) {
        return 0;
    }

    if (is_server_mode() && (!config_contains("server_url") || !config_contains("server_port"))) {
        std::cout << "error: server_mode=true needs a server_url value and a server_port value" << std::endl;

        return 0;
    }

    if (!has_enough_colors()) {
        std::cout << "WARNING: The terminal does not seem to have enough colors, some command may not work as intended" << std::endl;
    }

    // Restore the caches
    load_currency_cache();
    load_share_price_cache();

    if (!is_server_running() && is_server_mode()) {
        // 1. Ensure that the server is running

        auto res = budget::api_get("/server/up/", false);

        if(!res.success || res.result != "yes"){
            std::cout << "The server is not running, cannot run in server mode" << std::endl;
            return 0;
        }

        std::map<std::string, std::string> params;
        params["version"] = get_version_short();

        auto version_res = budget::api_post("/server/version/support/", params);

        if(!version_res.success || version_res.result != "yes"){
            std::cout << "The server does not support your version, cannot run in server mode" << std::endl;
            std::cout << "You should either upgrade the server or your client" << std::endl;
            return 0;
        }
    } else {
        auto old_data_version = to_number<size_t>(internal_config_value("data_version"));

        if (old_data_version > DATA_VERSION) {
            std::cout << "Unsupported database version, you should update budgetwarrior" << std::endl;

            return 0;
        }

        if (old_data_version < DATA_VERSION) {
            std::cout << "Migrate database..." << std::endl;

            if (old_data_version == 1 && DATA_VERSION >= 2) {
                migrate_recurring_1_to_2();
            }

            if (old_data_version <= 2 && DATA_VERSION >= 3) {
                migrate_wishes_2_to_3();
            }

            if (old_data_version <= 3 && DATA_VERSION >= 4) {
                //migrate_debts_3_to_4();
                migrate_wishes_3_to_4();
            }

            if (old_data_version <= 4 && DATA_VERSION >= 5) {
                migrate_assets_4_to_5();
            }

            if (old_data_version <= 5 && DATA_VERSION >= 6) {
                migrate_assets_5_to_6();
            }

            internal_config_value("data_version") = to_string(DATA_VERSION);

            // We want to make sure the new data version is set in stone!
            save_config();

            std::cout << "done" << std::endl;
        }
    }

    int code = 0;

    try {
        //Run the correct module
        module_runner runner(std::move(args));
        cpp::for_each_tuple_t<modules_tuple>(runner);

        if (!runner.handled) {
            std::cout << "Unhandled command \"" << runner.args[0] << "\"" << std::endl;

            code = 1;
        }
    } catch (const budget_exception& exception) {
        std::cout << exception.message() << std::endl;

        code = 2;
    }

    // Save the caches
    save_currency_cache();
    save_share_price_cache();

    save_config();

    return code;
}
