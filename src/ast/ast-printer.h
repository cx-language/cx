#pragma once

#include <memory>
#include <ostream>

namespace llvm {
template<typename T>
class ArrayRef;
}

namespace delta {

class Module;
class VarExpr;
class StringLiteralExpr;
class CharacterLiteralExpr;
class IntLiteralExpr;
class FloatLiteralExpr;
class BoolLiteralExpr;
class NullLiteralExpr;
class UndefinedLiteralExpr;
class ArrayLiteralExpr;
class TupleExpr;
class UnaryExpr;
class BinaryExpr;
class CallExpr;
class CastExpr;
class SizeofExpr;
class AddressofExpr;
class MemberExpr;
class SubscriptExpr;
class UnwrapExpr;
class LambdaExpr;
class IfExpr;
class Expr;
class stream;
class ReturnStmt;
class VarStmt;
class DeferStmt;
class IfStmt;
class SwitchStmt;
class WhileStmt;
class ForStmt;
class BreakStmt;
class ContinueStmt;
class CompoundStmt;
class Stmt;
class ParamDecl;
class GenericParamDecl;
class FunctionDecl;
class InitDecl;
class DeinitDecl;
class FunctionTemplate;
class FieldDecl;
class TypeDecl;
class TypeTemplate;
class VarDecl;
class ImportDecl;
class Decl;

class ASTPrinter {
public:
    ASTPrinter(std::ostream& out) : out(out), indentLevel(0) {}
    void printModule(const Module& module);
    void printVarExpr(const VarExpr& expr);
    void printStringLiteralExpr(const StringLiteralExpr& expr);
    void printCharacterLiteralExpr(const CharacterLiteralExpr& expr);
    void printIntLiteralExpr(const IntLiteralExpr& expr);
    void printFloatLiteralExpr(const FloatLiteralExpr& expr);
    void printBoolLiteralExpr(const BoolLiteralExpr& expr);
    void printNullLiteralExpr(const NullLiteralExpr&);
    void printUndefinedLiteralExpr(const UndefinedLiteralExpr&);
    void printArrayLiteralExpr(const ArrayLiteralExpr& expr);
    void printTupleExpr(const TupleExpr& expr);
    void printUnaryExpr(const UnaryExpr& expr);
    void printBinaryExpr(const BinaryExpr& expr);
    void printCallExpr(const CallExpr& expr);
    void printCastExpr(const CastExpr& expr);
    void printSizeofExpr(const SizeofExpr& expr);
    void printAddressofExpr(const AddressofExpr& expr);
    void printMemberExpr(const MemberExpr& expr);
    void printSubscriptExpr(const SubscriptExpr& expr);
    void printUnwrapExpr(const UnwrapExpr& expr);
    void printLambdaExpr(const LambdaExpr& expr);
    void printIfExpr(const IfExpr& expr);
    void printExpr(const Expr& expr);
    void printReturnStmt(const ReturnStmt& stmt);
    void printVarStmt(const VarStmt& stmt);
    void printDeferStmt(const DeferStmt& stmt);
    void printIfStmt(const IfStmt& stmt);
    void printSwitchStmt(const SwitchStmt& stmt);
    void printWhileStmt(const WhileStmt& stmt);
    void printForStmt(const ForStmt& stmt);
    void printBreakStmt(const BreakStmt&);
    void printContinueStmt(const ContinueStmt&);
    void printCompoundStmt(const CompoundStmt& stmt);
    void printStmt(const Stmt& stmt);
    void printParamDecl(const ParamDecl& decl);
    void printFunctionDecl(const FunctionDecl& decl);
    void printInitDecl(const InitDecl& decl);
    void printDeinitDecl(const DeinitDecl& decl);
    void printFunctionTemplate(const FunctionTemplate& decl);
    void printFieldDecl(const FieldDecl& decl);
    void printTypeDecl(const TypeDecl& decl);
    void printTypeTemplate(const TypeTemplate& decl);
    void printVarDecl(const VarDecl& decl);
    void printImportDecl(const ImportDecl& decl);
    void printDecl(const Decl& decl);

private:
    void breakLine();
    void printParams(llvm::ArrayRef<ParamDecl> params);
    void printGenericParams(llvm::ArrayRef<GenericParamDecl> genericParams);
    void printBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> block);

private:
    std::ostream& out;
    int indentLevel;
};

} // namespace delta
