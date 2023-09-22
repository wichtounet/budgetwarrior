//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <string>
#include <iostream>
#include <tuple>
#include <unordered_map>

#include "cpp_utils/tmp.hpp"

#include "config.hpp"
#include "args.hpp"
#include "budget_exception.hpp"
#include "api.hpp"
#include "currency.hpp"
#include "share.hpp"
#include "logging.hpp"
#include "data.hpp"

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
#include "retirement.hpp"

using namespace budget;

namespace {

using modules_tuple = std::tuple<
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
            budget::help_module
    >;

template <typename Module>
concept needs_loading = requires(Module module) {
    module.load();
};

template <typename Module>
concept needs_unloading = requires(Module module) {
    module.unload();
};

template <typename Module>
concept needs_preloading = requires(Module module) {
    module.preload();
};

template <typename Module>
concept has_disable_preloading_field = requires(Module module) {
    module_traits<Module>::disable_preloading;
};

template <typename Module>
concept has_aliases_field = requires {
    module_traits<Module>::aliases;
};

template<typename Module>
constexpr bool disable_preloading() {
    if constexpr (has_disable_preloading_field<Module>) {
        return module_traits<Module>::disable_preloading;
    } else {
        return false;
    }
}

struct module_loader {
    template<typename Module>
    inline void operator()(){
        if constexpr (needs_preloading<Module>) {
            Module module;
            module.preload();
        }
    }
};

struct module_runner {
    std::vector<std::string> args;
    bool handled = false;

    explicit module_runner(std::vector<std::string>&& args) : args(std::forward<std::vector<std::string>>(args)) {
        //Nothing to init
    }

    template<typename Module>
    inline void handle_module(){
        //Preload each module that needs it
        if constexpr (!disable_preloading<Module>()) {
            module_loader loader;
            cpp::for_each_tuple_t<modules_tuple>(loader);
        }

        Module module;

        if constexpr (needs_loading<Module>) {
            module.load();
        }

        module.handle(args);

        if constexpr (needs_unloading<Module>) {
            module.unload();
        }

        handled = true;
    }

    template<typename Module>
    inline void operator()(){
        if(handled){
            return;
        }

        if(args.empty()){
            if constexpr (module_traits<Module>::is_default){
                handle_module<Module>();
            }
        } else if(args[0] == module_traits<Module>::command){
            handle_module<Module>();
        }
    }
};

struct aliases_collector {
    std::vector<std::pair<const char*, const char*>> aliases;

    template <typename Module>
    inline void operator()() {
        if constexpr (has_aliases_field<Module>) {
            for (auto& v : module_traits<Module>::aliases) {
                aliases.push_back(v);
            }
        }
    }
};

std::string exec_command(const std::string& command) {
    std::stringstream output;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != nullptr) {
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
    const std::locale global_locale("");
    std::locale::global(global_locale);

    budget::init_logging(argc, const_cast<char**>(argv));

    //Collect all aliases
    aliases_collector collector;
    cpp::for_each_tuple_t<modules_tuple>(collector);

    //Parse the command line args
    auto args = parse_args(argc, argv, collector.aliases);

    if (!load_config()) {
        LOG_F(ERROR, "Unable to load the configuration");
        return 0;
    }

    if (is_server_mode() && (!config_contains("server_url") || !config_contains("server_port"))) {
        LOG_F(ERROR, "server_mode=true needs a server_url value and a server_port value");

        return 0;
    }

    if (!has_enough_colors()) {
        LOG_F(WARNING, "The terminal does not seem to have enough colors, some command may not work as intended");
    }

    // Restore the caches
    load_currency_cache();
    load_share_price_cache();

    if (is_server_mode()) {
        // 1. Ensure that the server is running

        auto res = budget::api_get("/server/up/");

        if(!res.success || res.result != "yes"){
            LOG_F(ERROR, "The server is not running, cannot run in server mode");
            return 0;
        }

        std::map<std::string, std::string, std::less<>> params;
        params["version"] = get_version_short();

        auto version_res = budget::api_post("/server/version/support/", params);

        if(!version_res.success || version_res.result != "yes"){
            LOG_F(ERROR, "The server does not support your version, cannot run in server mode");
            LOG_F(ERROR, "You should either upgrade the server or your client");
            return 0;
        }
    } else {
        auto old_data_version = to_number<size_t>(internal_config_value("data_version"));

        if (!budget::migrate_database(old_data_version)) {
            return 0;
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
        // TODO We should be able to differentiate between real errors and
        // command line errors
        std::cout << exception.message() << std::endl;

        code = 2;
    }

    // Save the caches
    save_currency_cache();
    save_share_price_cache();

    save_config();

    return code;
}
