#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_set>
#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>
#include "codegen.h"

static std::ostream* out = nullptr;

/// Points to the function whose body is currently being generated.
static boost::optional<std::string> currentFunc = boost::none;

/// Array types that have been defined so far.
static std::unordered_set<std::string> definedArrayTypes;

std::string toC(const Type& type) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            boost::string_ref name = type.getBasicType().name;
            if (name == "int") return "int";
            if (name == "uint") return "unsigned";
            if (name == "int8") return "int8_t";
            if (name == "int16") return "int16_t";
            if (name == "int32") return "int32_t";
            if (name == "int64") return "int64_t";
            if (name == "uint8") return "uint8_t";
            if (name == "uint16") return "uint16_t";
            if (name == "uint32") return "uint32_t";
            if (name == "uint64") return "uint64_t";
            return std::string(name);
        }
        case TypeKind::ArrayType: {
            std::string arrayTypeCName = std::to_string(type.getArrayType().size)
                                         + toC(*type.getArrayType().elementType);
            if (definedArrayTypes.count(arrayTypeCName) > 0) {
                return "struct __" + arrayTypeCName;
            }
            definedArrayTypes.insert(arrayTypeCName);
            return "struct __" + arrayTypeCName + "{"
                + toC(*type.getArrayType().elementType)
                + " a[" + std::to_string(type.getArrayType().size) + "];}";
        }
        case TypeKind::TupleType: {
            std::string string = "struct{";
            int index = 0;
            for (const Type& subtype : type.getTupleType().subtypes) {
                string += toC(subtype) + " _" + std::to_string(index++) + ";";
            }
            return string + "}";
        }
        case TypeKind::FuncType:
            abort(); // TODO
        case TypeKind::PtrType:
            return toC(*type.getPtrType().pointeeType) + "*";
    }
}

void codegen(const Expr& expr);
void codegen(const Stmt& stmt);

void codegen(const VariableExpr& expr) {
    *out << expr.identifier;
}

void codegen(const StrLiteralExpr& expr) {
    *out << '"' << expr.value << '"';
}

void codegen(const IntLiteralExpr& expr) {
    *out << expr.value;
}

void codegen(const BoolLiteralExpr& expr) {
    *out << (expr.value ? "true" : "false");
}

void codegen(const PrefixExpr& expr) {
    *out << "(" << expr.op;
    codegen(*expr.operand);
    *out << ")";
}

void codegen(const BinaryExpr& expr) {
    *out << "(";
    codegen(*expr.left);
    *out << expr.op;
    codegen(*expr.right);
    *out << ")";
}

void codegen(const CallExpr& expr) {
    if (expr.isInitializerCall) {
        *out << "__init_" << expr.funcName << "(";
    } else if (expr.isMemberFuncCall()) {
        *out << "__M" << expr.receiver->getType().getBasicType().name << "$" << expr.funcName << "(&";
        codegen(*expr.receiver);
        if (!expr.args.empty()) *out << ",";
    } else {
        *out << expr.funcName << "(";
    }
    for (const Arg& arg : expr.args) {
        codegen(*arg.value);
        if (&arg != &expr.args.back()) *out << ",";
    }
    *out << ")";
}

void codegen(const CastExpr& expr) {
    *out << "((" << toC(expr.type) << ")(";
    codegen(*expr.expr);
    *out << "))";
}

void codegen(const MemberExpr& expr) {
    if (expr.base == "this" && currentFunc->find("__init_") != 0) *out << "this->";
    else *out << expr.base << ".";
    *out << expr.member;
}

void codegen(const SubscriptExpr& expr) {
    codegen(*expr.array);
    *out << (expr.array->getType().getKind() == TypeKind::PtrType ? "->" : ".") << "a[";
    codegen(*expr.index);
    *out << "]";
}

void codegen(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:   codegen(expr.getVariableExpr()); break;
        case ExprKind::StrLiteralExpr: codegen(expr.getStrLiteralExpr()); break;
        case ExprKind::IntLiteralExpr: codegen(expr.getIntLiteralExpr()); break;
        case ExprKind::BoolLiteralExpr:codegen(expr.getBoolLiteralExpr()); break;
        case ExprKind::PrefixExpr:     codegen(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr:     codegen(expr.getBinaryExpr()); break;
        case ExprKind::CallExpr:       codegen(expr.getCallExpr()); break;
        case ExprKind::CastExpr:       codegen(expr.getCastExpr()); break;
        case ExprKind::MemberExpr:     codegen(expr.getMemberExpr()); break;
        case ExprKind::SubscriptExpr:  codegen(expr.getSubscriptExpr()); break;
    }
}

void codegen(const ReturnStmt& stmt) {
    *out << "return ";
    if (stmt.values.size() == 1) {
        codegen(stmt.values[0]);
    } else if (stmt.values.size() > 1) {
        *out << "(__typeof__(" << *currentFunc << "())){";
        for (const Expr& expr : stmt.values) {
            codegen(expr);
            if (&expr != &stmt.values.back()) *out << ",";
        }
        *out << "}";
    }
    *out << ";";
}

void codegen(const VarDecl& decl);

void codegen(const VariableStmt& stmt) {
    codegen(*stmt.decl);
}

void codegen(const IncrementStmt& stmt) {
    codegen(stmt.operand);
    *out << "++;";
}

void codegen(const DecrementStmt& stmt) {
    codegen(stmt.operand);
    *out << "--;";
}

void codegen(const IfStmt& ifStmt) {
    *out << "if(";
    codegen(ifStmt.condition);
    *out << "){";
    for (const Stmt& stmt : ifStmt.thenBody) {
        codegen(stmt);
    }
    *out << "}";
    if (!ifStmt.elseBody.empty()) {
        *out << "else{";
        for (const Stmt& stmt : ifStmt.elseBody) {
            codegen(stmt);
        }
        *out << "}";
    }
}

void codegen(const WhileStmt& whileStmt) {
    *out << "while(";
    codegen(whileStmt.condition);
    *out << "){";
    for (const Stmt& stmt : whileStmt.body) {
        codegen(stmt);
    }
    *out << "}";
}

void codegen(const AssignStmt& stmt) {
    codegen(stmt.lhs);
    *out << "=";
    codegen(stmt.rhs);
    *out << ";";
}

void codegen(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    codegen(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  codegen(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: codegen(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: codegen(stmt.getDecrementStmt()); break;
        case StmtKind::CallStmt:      codegen(stmt.getCallStmt().expr); *out << ";"; break;
        case StmtKind::IfStmt:        codegen(stmt.getIfStmt()); break;
        case StmtKind::WhileStmt:     codegen(stmt.getWhileStmt()); break;
        case StmtKind::AssignStmt:    codegen(stmt.getAssignStmt()); break;
    }
}

void codegen(const ParamDecl& decl) {
    *out << toC(decl.type) << " " << decl.name;
}

void codegenPrototype(const FuncDecl& decl) {
    *out << (decl.name == "main" ? "int" : toC(decl.returnType));
    if (decl.isMemberFunc()) {
        *out << " __M" << decl.receiverType << "$" <<  decl.name << "(" << decl.receiverType << "*this";
        if (!decl.params.empty()) *out << ",";
    } else {
        *out << " " << decl.name << "(";
    }
    for (const ParamDecl& param : decl.params) {
        codegen(param);
        if (&param != &decl.params.back()) *out << ",";
    }
    *out << ")";
}

void codegen(const FuncDecl& decl) {
    if (decl.isExtern()) return;
    codegenPrototype(decl);
    *out << "{";
    auto currentFuncBackup = currentFunc;
    currentFunc = decl.name;
    for (const Stmt& stmt : *decl.body) {
        codegen(stmt);
    }
    currentFunc = currentFuncBackup;
    if (decl.name == "main") *out << "return 0;";
    *out << "}";
}

void codegen(const InitDecl& decl) {
    *out << toC(decl.getTypeDecl().getType()) << " __init_" << decl.getTypeDecl().name << "(";
    for (const ParamDecl& param : decl.params) {
        codegen(param);
        if (&param != &decl.params.back()) *out << ",";
    }
    *out << "){";
    auto currentFuncBackup = currentFunc;
    currentFunc = "__init_" + decl.getTypeDecl().name;
    *out << decl.getTypeDecl().name << " this;";
    for (const Stmt& stmt : *decl.body) {
        codegen(stmt);
    }
    currentFunc = currentFuncBackup;
    *out << "return this;}";
}

void codegen(const TypeDecl& decl) {
    *out << "typedef struct " << decl.name << "{";
    for (const FieldDecl& field : decl.fields) {
        *out << toC(field.type) << " " << field.name << ";";
    }
    *out << "}" << decl.name << ";";
}

void codegen(const VarDecl& decl) {
    *out << toC(decl.getType()) << " " << decl.name;
    if (decl.initializer) {
        *out << "=";
        codegen(*decl.initializer);
    }
    *out << ";";
}

void codegen(const FieldDecl& decl) {
    *out << toC(decl.type) << " " << decl.name << ";";
}

void codegen(const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: codegen(decl.getParamDecl()); break;
        case DeclKind::FuncDecl:  codegen(decl.getFuncDecl()); break;
        case DeclKind::InitDecl:  codegen(decl.getInitDecl()); break;
        case DeclKind::TypeDecl:  codegen(decl.getTypeDecl()); break;
        case DeclKind::VarDecl:   codegen(decl.getVarDecl()); break;
        case DeclKind::FieldDecl: codegen(decl.getFieldDecl()); break;
    }
}

void cgen::compile(const std::vector<Decl>& decls, boost::string_ref outputPath) {
    std::ofstream outputFile;
    if (outputPath == "stdout") out = &std::cout;
    else {
        outputFile.open(outputPath.data());
        out = &outputFile;
    }

    *out << "#include <stdbool.h>\n#include <stdint.h>\n#include <stddef.h>\n";

    // Output struct definitions first.
    for (const Decl& decl : decls) {
        switch (decl.getKind()) {
            case DeclKind::TypeDecl:
                codegen(decl.getTypeDecl());
                break;
            case DeclKind::FuncDecl: case DeclKind::ParamDecl: case DeclKind::VarDecl:
            case DeclKind::FieldDecl: case DeclKind::InitDecl:
                break;
        }
    }

    // Output function declarations.
    for (const Decl& decl : decls) {
        switch (decl.getKind()) {
            case DeclKind::FuncDecl:
                codegenPrototype(decl.getFuncDecl());
                *out << ";";
                break;
            case DeclKind::TypeDecl: case DeclKind::ParamDecl: case DeclKind::VarDecl:
            case DeclKind::FieldDecl: case DeclKind::InitDecl:
                break;
        }
    }

    for (const Decl& decl : decls) {
        if (decl.getKind() != DeclKind::TypeDecl) { // Struct definitions have already been emitted.
            codegen(decl);
        }
    }
}
