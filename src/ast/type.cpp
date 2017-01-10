#include "type.h"

Type::Type(std::string name) : data(std::move(name)) { }

Type::Type(std::vector<Type> names) {
    if (names.size() == 1)  {
        data = std::move(names[0].getName());
    } else {
        data = std::move(names);
    }
}

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

bool operator==(const Type& lhs, const Type& rhs) {
    if (lhs.isTuple() != rhs.isTuple()) return false;
    if (lhs.isTuple()) {
        return lhs.getNames() == rhs.getNames();
    } else {
        return lhs.getName() == rhs.getName();
    }
}

bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Type& type) {
    if (type.isTuple()) {
        out << "(";
        for (const Type& memberType : type.getNames()) {
            out << memberType;
            if (&memberType != &type.getNames().back()) out << " ";
        }
        out << ")";
    } else {
        out << type.getName();
    }
    return out;
}
