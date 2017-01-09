#pragma once

#include <vector>
#include <boost/utility/string_ref.hpp>
#include "../ast/decl.h"

namespace cgen {
    void compile(const std::vector<Decl>& decls, boost::string_ref fileName);
}
