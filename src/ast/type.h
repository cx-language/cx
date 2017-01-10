#pragma once

#include <vector>
#include <string>
#include <memory>
#include <ostream>
#include <boost/variant.hpp>

enum class TypeKind {
    BasicType,
    TupleType,
    FuncType,
};

class Type;

struct FuncType {
    std::vector<Type> returnTypes;
    std::vector<Type> paramTypes;
};

class Type {
public:
    /// Creates a non-tuple type with the given type name.
    explicit Type(std::string name);

    /// Creates a tuple type containing the given types.
    explicit Type(std::vector<Type> names);

    void appendType(std::string name);
    bool isTuple() const;
    std::string& getName();
    const std::string& getName() const;
    std::vector<Type>& getNames();
    const std::vector<Type>& getNames() const;

#define DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Type(KIND&& value) : data(std::move(value)) { } \
    \
    KIND& get##KIND() { \
        assert(getKind() == TypeKind::KIND); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(getKind() == TypeKind::KIND); \
        return boost::get<KIND>(data); \
    }
    DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(FuncType)
#undef DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR

    TypeKind getKind() const { return static_cast<TypeKind>(data.which()); }

private:
    boost::variant<std::string, std::vector<Type>, FuncType> data;
};

bool operator==(const Type&, const Type&);
bool operator!=(const Type&, const Type&);
std::ostream& operator<<(std::ostream&, const Type&);
