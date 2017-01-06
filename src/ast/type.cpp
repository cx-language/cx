#include "type.h"

Type::Type(std::string name) : data(std::move(name)) { }

Type::Type(std::vector<Type> names) : data(std::move(names)) { }

void Type::appendType(std::string name) {
    if (!isTuple()) {
        std::string firstTypeName = std::move(getName());
        data = std::vector<Type>();
        getNames().emplace_back(std::move(firstTypeName));
    }
    getNames().emplace_back(std::move(name));
}

bool Type::isTuple() const { return data.which() == 1; }

std::string& Type::getName() {
    assert(!isTuple());
    return boost::get<std::string>(data);
}

const std::string& Type::getName() const {
    assert(!isTuple());
    return boost::get<std::string>(data);
}

std::vector<Type>& Type::getNames() {
    assert(isTuple());
    return boost::get<std::vector<Type>>(data);
}

const std::vector<Type>& Type::getNames() const {
    assert(isTuple());
    return boost::get<std::vector<Type>>(data);
}
