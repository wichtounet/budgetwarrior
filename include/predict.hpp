//=======================================================================
// Copyright (c) 2013-2020 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <vector>
#include <string>
#include <array>

#include "module_traits.hpp"

namespace budget {

struct predict_module {
    void load() const;
    void handle(const std::vector<std::string>& args) const;
};

template<>
struct module_traits<predict_module> {
    static constexpr const bool is_default = false;
    static constexpr const char* command = "predict";
};

} //end of namespace budget
