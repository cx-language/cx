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
    PtrType,
};

class Type;

struct BasicType {
    std::string name;
};

struct TupleType {
    std::vector<Type> subtypes;
};

struct FuncType {
    std::vector<Type> returnTypes;
    std::vector<Type> paramTypes;
};

struct PtrType {
    std::unique_ptr<Type> pointeeType;
    PtrType(std::unique_ptr<Type> type) : pointeeType(std::move(type)) { }
    PtrType(const PtrType&);
    PtrType& operator=(const PtrType&);
};

class Type {
public:
#define DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Type(KIND&& value) : data(std::move(value)) { \
        if (TypeKind::KIND == TypeKind::TupleType && getTupleType().subtypes.size() == 1) {\
            auto type = std::move(getTupleType().subtypes[0]); \
            *this = std::move(type); \
        } \
    } \
    \
    KIND& get##KIND() { \
        assert(getKind() == TypeKind::KIND); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(getKind() == TypeKind::KIND); \
        return boost::get<KIND>(data); \
    }
    DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(BasicType)
    DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(TupleType)
    DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(FuncType)
    DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR(PtrType)
#undef DEFINE_TYPEKIND_GETTER_AND_CONSTRUCTOR

    void appendType(Type type);
    TypeKind getKind() const { return static_cast<TypeKind>(data.which()); }

private:
    boost::variant<BasicType, TupleType, FuncType, PtrType> data;
};

bool operator==(const Type&, const Type&);
bool operator!=(const Type&, const Type&);
std::ostream& operator<<(std::ostream&, const Type&);
