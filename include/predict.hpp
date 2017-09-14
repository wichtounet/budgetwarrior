//=======================================================================
// Copyright (c) 2013-2017 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef PREDICT_H
#define PREDICT_H

#include <vector>
#include <string>
#include <array>

#include "module_traits.hpp"

namespace budget {

struct predict_module {
    void load();
    void handle(std::vector<std::string>& args);
};

template<>
struct module_traits<predict_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "predict";
};

} //end of namespace budget

#endif
