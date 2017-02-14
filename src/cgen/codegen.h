#pragma once

#include <vector>
#include <boost/utility/string_ref.hpp>
#include "../ast/decl.h"

namespace delta {

namespace cgen {
    void compile(const std::vector<Decl>& decls, boost::string_ref fileName);
}

}
