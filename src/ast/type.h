#pragma once

#include <vector>
#include <string>
#include <boost/variant.hpp>

class Type {
public:
    /// Creates a non-tuple type with the given type name.
    explicit Type(std::string name);

    /// Creates a tuple type containing the given types.
    Type(std::vector<Type> names);

    void appendType(std::string name);
    bool isTuple() const;
    std::string& getName();
    const std::string& getName() const;
    std::vector<Type>& getNames();
    const std::vector<Type>& getNames() const;

private:
    boost::variant<std::string, std::vector<Type>> data;
};

bool operator==(const Type&, const Type&);
bool operator!=(const Type&, const Type&);
