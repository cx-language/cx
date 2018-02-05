#include <iostream>
#include <string>
#pragma warning(push, 0)
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "ast-printer.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"
#include "../ast/module.h"
#include "../support/utility.h"

using namespace delta;

/// Used to insert a line break followed by appropriate indentation.
void ASTPrinter::breakLine() {
    out << '\n';
    for (int i = 0; i < indentLevel; ++i) {
        out << "    ";
    }
}

void ASTPrinter::printParams(llvm::ArrayRef<ParamDecl> params) {
    out << "(";
    for (const ParamDecl& param : params) {
        printParamDecl(param);
        if (&param != &params.back()) out << " ";
    }
    out << ")";
}

void ASTPrinter::printGenericParams(llvm::ArrayRef<GenericParamDecl> genericParams) {
    out << "(";
    for (const GenericParamDecl& genericParam : genericParams) {
        out << genericParam.getName();
        if (&genericParam != &genericParams.back()) out << " ";
    }
    out << ")";
}

void ASTPrinter::printVarExpr(const VarExpr& expr) {
    out << expr.getIdentifier();
}

void ASTPrinter::printStringLiteralExpr(const StringLiteralExpr& expr) {
    out << '"' << expr.getValue() << '"';
}

void ASTPrinter::printCharacterLiteralExpr(const CharacterLiteralExpr& expr) {
    out << '\'' << expr.getValue() << '\'';
}

void ASTPrinter::printIntLiteralExpr(const IntLiteralExpr& expr) {
    out << expr.getValue();
}

void ASTPrinter::printFloatLiteralExpr(const FloatLiteralExpr& expr) {
    out << expr.getValue();
}

void ASTPrinter::printBoolLiteralExpr(const BoolLiteralExpr& expr) {
    out << (expr.getValue() ? "true" : "false");
}

void ASTPrinter::printNullLiteralExpr(const NullLiteralExpr&) {
    out << "null";
}

void ASTPrinter::printArrayLiteralExpr(const ArrayLiteralExpr& expr) {
    out << "(array-literal";
    for (auto& e : expr.getElements()) {
        out << " ";
        printExpr(*e);
    }
    out << ")";
}

void ASTPrinter::printTupleExpr(const TupleExpr& expr) {
    out << "(tuple-literal";
    for (auto& e : expr.getElements()) {
        out << " ";
        printExpr(*e);
    }
    out << ")";
}

void ASTPrinter::printPrefixExpr(const PrefixExpr& expr) {
    out << "(" << expr.getOperator();
    printExpr(expr.getOperand());
    out << ")";
}

void ASTPrinter::printBinaryExpr(const BinaryExpr& expr) {
    out << "(";
    printExpr(expr.getLHS());
    out << " " << expr.getOperator() << " ";
    printExpr(expr.getRHS());
    out << ")";
}

void ASTPrinter::printCallExpr(const CallExpr& expr) {
    out << "(call ";
    printExpr(expr.getCallee());
    for (const Argument& arg : expr.getArgs()) {
        out << " ";
        printExpr(*arg.getValue());
    }
    out << ")";
}

void ASTPrinter::printCastExpr(const CastExpr& expr) {
    out << "(cast ";
    printExpr(expr.getExpr());
    out << " " << expr.getType() << ")";
}

void ASTPrinter::printSizeofExpr(const SizeofExpr& expr) {
    out << "(sizeof " << expr.getType() << ")";
}

void ASTPrinter::printAddressofExpr(const AddressofExpr& expr) {
    out << "(addressof ";
    printExpr(expr.getOperand());
    out << ")";
}

void ASTPrinter::printMemberExpr(const MemberExpr& expr) {
    out << "(member-expr ";
    printExpr(*expr.getBaseExpr());
    out << " " << expr.getMemberName() << ")";
}

void ASTPrinter::printSubscriptExpr(const SubscriptExpr& expr) {
    out << "(subscript ";
    printExpr(*expr.getBaseExpr());
    out << " ";
    printExpr(*expr.getIndexExpr());
    out << ")";
}

void ASTPrinter::printUnwrapExpr(const UnwrapExpr& expr) {
    out << "(unwrap ";
    printExpr(expr.getOperand());
    out << ")";
}

void ASTPrinter::printLambdaExpr(const LambdaExpr& expr) {
    out << "(lambda ";
    printParams(expr.getParams());
    out << " ";
    printExpr(*expr.getBody());
    out << ")";
}

void ASTPrinter::printIfExpr(const IfExpr& expr) {
    out << "(if-expr ";
    printExpr(*expr.getCondition());
    out << " ";
    printExpr(*expr.getThenExpr());
    out << " ";
    printExpr(*expr.getElseExpr());
    out << ")";
}

void ASTPrinter::printExpr(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VarExpr: printVarExpr(llvm::cast<VarExpr>(expr)); break;
        case ExprKind::StringLiteralExpr: printStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr)); break;
        case ExprKind::CharacterLiteralExpr: printCharacterLiteralExpr(llvm::cast<CharacterLiteralExpr>(expr)); break;
        case ExprKind::IntLiteralExpr: printIntLiteralExpr(llvm::cast<IntLiteralExpr>(expr)); break;
        case ExprKind::FloatLiteralExpr: printFloatLiteralExpr(llvm::cast<FloatLiteralExpr>(expr)); break;
        case ExprKind::BoolLiteralExpr: printBoolLiteralExpr(llvm::cast<BoolLiteralExpr>(expr)); break;
        case ExprKind::NullLiteralExpr: printNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr)); break;
        case ExprKind::ArrayLiteralExpr: printArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr)); break;
        case ExprKind::TupleExpr: printTupleExpr(llvm::cast<TupleExpr>(expr)); break;
        case ExprKind::PrefixExpr: printPrefixExpr(llvm::cast<PrefixExpr>(expr)); break;
        case ExprKind::BinaryExpr: printBinaryExpr(llvm::cast<BinaryExpr>(expr)); break;
        case ExprKind::CallExpr: printCallExpr(llvm::cast<CallExpr>(expr)); break;
        case ExprKind::CastExpr: printCastExpr(llvm::cast<CastExpr>(expr)); break;
        case ExprKind::SizeofExpr: printSizeofExpr(llvm::cast<SizeofExpr>(expr)); break;
        case ExprKind::AddressofExpr: printAddressofExpr(llvm::cast<AddressofExpr>(expr)); break;
        case ExprKind::MemberExpr: printMemberExpr(llvm::cast<MemberExpr>(expr)); break;
        case ExprKind::SubscriptExpr: printSubscriptExpr(llvm::cast<SubscriptExpr>(expr)); break;
        case ExprKind::UnwrapExpr: printUnwrapExpr(llvm::cast<UnwrapExpr>(expr)); break;
        case ExprKind::LambdaExpr: printLambdaExpr(llvm::cast<LambdaExpr>(expr)); break;
        case ExprKind::IfExpr: printIfExpr(llvm::cast<IfExpr>(expr)); break;
    }
}

void ASTPrinter::printBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> block) {
    indentLevel++;
    for (const auto& stmt : block) {
        printStmt(*stmt);
    }
    indentLevel--;
}

void ASTPrinter::printReturnStmt(const ReturnStmt& stmt) {
    breakLine();
    out << "(return-stmt";

    if (stmt.getReturnValue()) {
        out << " ";
        printExpr(*stmt.getReturnValue());
    }

    out << ")";
}

void ASTPrinter::printVarStmt(const VarStmt& stmt) {
    breakLine();
    out << "(var-stmt " << stmt.getDecl().getName() << " ";
    if (stmt.getDecl().getInitializer()) {
        printExpr(*stmt.getDecl().getInitializer());
    } else {
        out << "undefined";
    }
    out << ")";
}

void ASTPrinter::printIncrementStmt(const IncrementStmt& stmt) {
    breakLine();
    out << "(inc-stmt ";
    printExpr(stmt.getOperand());
    out << ")";
}

void ASTPrinter::printDecrementStmt(const DecrementStmt& stmt) {
    breakLine();
    out << "(dec-stmt ";
    printExpr(stmt.getOperand());
    out << ")";
}

void ASTPrinter::printDeferStmt(const DeferStmt& stmt) {
    breakLine();
    out << "(defer-stmt ";
    printExpr(stmt.getExpr());
    out << ")";
}

void ASTPrinter::printIfStmt(const IfStmt& stmt) {
    breakLine();
    out << "(if-stmt ";
    printExpr(stmt.getCondition());
    indentLevel++;
    breakLine();
    out << "(then";
    printBlock(stmt.getThenBody());
    out << ")";
    if (!stmt.getElseBody().empty()) {
        breakLine();
        out << "(else";
        indentLevel++;
        for (const auto& substmt : stmt.getElseBody()) {
            if (!substmt->isIfStmt()) breakLine();
            printStmt(*substmt);
        }
        indentLevel--;
        out << ")";
    }
    indentLevel--;
    out << ")";
}

void ASTPrinter::printSwitchStmt(const SwitchStmt& stmt) {
    breakLine();
    out << "(switch-stmt ";
    printExpr(stmt.getCondition());
    indentLevel++;
    for (const SwitchCase& switchCase : stmt.getCases()) {
        breakLine();
        out << "(case ";
        printExpr(*switchCase.getValue());
        printBlock(switchCase.getStmts());
        out << ")";
    }
    indentLevel--;
    out << ")";
}

void ASTPrinter::printWhileStmt(const WhileStmt& stmt) {
    breakLine();
    out << "(while-stmt ";
    printExpr(stmt.getCondition());
    printBlock(stmt.getBody());
    out << ")";
}

void ASTPrinter::printForStmt(const ForStmt& stmt) {
    breakLine();
    out << "(for-stmt ";
    printVarDecl(*stmt.getVariable());
    out << " ";
    printExpr(stmt.getRangeExpr());
    printBlock(stmt.getBody());
    out << ")";
}

void ASTPrinter::printBreakStmt(const BreakStmt&) {
    breakLine();
    out << "(break-stmt)";
}

void ASTPrinter::printContinueStmt(const ContinueStmt&) {
    breakLine();
    out << "(continue-stmt)";
}

void ASTPrinter::printAssignStmt(const AssignStmt& stmt) {
    if (stmt.isCompoundAssignment()) {
        breakLine();
        auto& binaryExpr = llvm::cast<BinaryExpr>(*stmt.getRHS());
        out << "(compound-assign-stmt " << binaryExpr.getOperator() << " ";
        printExpr(binaryExpr.getLHS());
        out << " ";
        printExpr(binaryExpr.getRHS());
        out << ")";
    } else {
        breakLine();
        out << "(assign-stmt ";
        printExpr(*stmt.getLHS());
        out << " ";
        printExpr(*stmt.getRHS());
        out << ")";
    }
}

void ASTPrinter::printCompoundStmt(const CompoundStmt& stmt) {
    breakLine();
    out << "(compound-stmt ";
    printBlock(stmt.getBody());
    out << ")";
}

void ASTPrinter::printStmt(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt: printReturnStmt(llvm::cast<ReturnStmt>(stmt)); break;
        case StmtKind::VarStmt: printVarStmt(llvm::cast<VarStmt>(stmt)); break;
        case StmtKind::IncrementStmt: printIncrementStmt(llvm::cast<IncrementStmt>(stmt)); break;
        case StmtKind::DecrementStmt: printDecrementStmt(llvm::cast<DecrementStmt>(stmt)); break;
        case StmtKind::ExprStmt: breakLine(); printExpr(llvm::cast<ExprStmt>(stmt).getExpr()); break;
        case StmtKind::DeferStmt: printDeferStmt(llvm::cast<DeferStmt>(stmt)); break;
        case StmtKind::IfStmt: printIfStmt(llvm::cast<IfStmt>(stmt)); break;
        case StmtKind::SwitchStmt: printSwitchStmt(llvm::cast<SwitchStmt>(stmt)); break;
        case StmtKind::WhileStmt: printWhileStmt(llvm::cast<WhileStmt>(stmt)); break;
        case StmtKind::ForStmt: printForStmt(llvm::cast<ForStmt>(stmt)); break;
        case StmtKind::BreakStmt: printBreakStmt(llvm::cast<BreakStmt>(stmt)); break;
        case StmtKind::ContinueStmt: printContinueStmt(llvm::cast<ContinueStmt>(stmt)); break;
        case StmtKind::AssignStmt: printAssignStmt(llvm::cast<AssignStmt>(stmt)); break;
        case StmtKind::CompoundStmt: printCompoundStmt(llvm::cast<CompoundStmt>(stmt)); break;
    }
}

void ASTPrinter::printParamDecl(const ParamDecl& decl) {
    out << "(" << decl.getType() << " " << decl.getName() << ")";
}

void ASTPrinter::printFunctionDecl(const FunctionDecl& decl) {
    breakLine();
    out << (decl.isExtern() ? "(extern-function-decl " : "(function-decl ");
    delta::operator<<(out, decl.getName());
    out << " ";
    printParams(decl.getParams());
    out << " " << decl.getReturnType();
    if (!decl.isExtern()) {
        printBlock(decl.getBody());
    }
    out << ")";
}

void ASTPrinter::printInitDecl(const InitDecl& decl) {
    breakLine();
    out << "(init-decl " << decl.getTypeDecl()->getName() << " (";
    for (const ParamDecl& param : decl.getParams()) {
        printParamDecl(param);
        if (&param != &decl.getParams().back()) out << " ";
    }
    out << ")";
    printBlock(decl.getBody());
    out << ")";
}

void ASTPrinter::printDeinitDecl(const DeinitDecl& decl) {
    breakLine();
    out << "(deinit-decl " << decl.getTypeDecl()->getName();
    printBlock(decl.getBody());
    out << ")";
}

void ASTPrinter::printFunctionTemplate(const FunctionTemplate& decl) {
    breakLine();
    out << "(function-template ";
    printGenericParams(decl.getGenericParams());
    out << decl.getFunctionDecl() << ")";
}

void ASTPrinter::printFieldDecl(const FieldDecl& decl) {
    breakLine();
    out << "(field-decl " << decl.getType() << " " << decl.getName() << ")";
}

void ASTPrinter::printTypeDecl(const TypeDecl& decl) {
    breakLine();
    out << "(type-decl ";
    switch (decl.getTag()) {
        case TypeTag::Struct: out << "struct "; break;
        case TypeTag::Class: out << "class "; break;
        case TypeTag::Interface: out << "interface "; break;
        case TypeTag::Union: out << "union "; break;
        case TypeTag::Enum: out << "enum "; break;
    }
    out << decl.getName();
    indentLevel++;

    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(&decl)) {
        for (auto& enumCase : enumDecl->getCases()) {
            breakLine();
            out << "(enum-case " << enumCase.getName() << " ";
            printExpr(*enumCase.getValue());
            out << ")";
        }
    }

    for (const FieldDecl& field : decl.getFields()) {
        printFieldDecl(field);
    }

    indentLevel--;
    out << ")";
}

void ASTPrinter::printTypeTemplate(const TypeTemplate& decl) {
    breakLine();
    out << "(type-template ";
    printGenericParams(decl.getGenericParams());
    out << " ";
    printTypeDecl(*decl.getTypeDecl());
    out << ")";
}

void ASTPrinter::printVarDecl(const VarDecl& decl) {
    breakLine();
    out << "(var-decl " << decl.getName() << " ";
    printExpr(*decl.getInitializer());
    out << ")";
}

void ASTPrinter::printImportDecl(const ImportDecl& decl) {
    breakLine();
    out << "(import-decl \"" << decl.getTarget() << "\")";
}

void ASTPrinter::printDecl(const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: printParamDecl(llvm::cast<ParamDecl>(decl)); break;
        case DeclKind::GenericParamDecl: llvm_unreachable("handled via FunctionTemplate"); break;
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl: printFunctionDecl(llvm::cast<FunctionDecl>(decl)); break;
        case DeclKind::InitDecl: printInitDecl(llvm::cast<InitDecl>(decl)); break;
        case DeclKind::DeinitDecl: printDeinitDecl(llvm::cast<DeinitDecl>(decl)); break;
        case DeclKind::FunctionTemplate: printFunctionTemplate(llvm::cast<FunctionTemplate>(decl)); break;
        case DeclKind::TypeDecl: printTypeDecl(llvm::cast<TypeDecl>(decl)); break;
        case DeclKind::TypeTemplate: printTypeTemplate(llvm::cast<TypeTemplate>(decl)); break;
        case DeclKind::EnumDecl: printTypeDecl(llvm::cast<EnumDecl>(decl)); break;
        case DeclKind::VarDecl: printVarDecl(llvm::cast<VarDecl>(decl)); break;
        case DeclKind::FieldDecl: printFieldDecl(llvm::cast<FieldDecl>(decl)); break;
        case DeclKind::ImportDecl: printImportDecl(llvm::cast<ImportDecl>(decl)); break;
    }
}

void ASTPrinter::printModule(const Module& module) {
    for (const auto& sourceFile : module.getSourceFiles()) {
        out << "(source-file " << sourceFile.getFilePath();
        indentLevel++;
        for (const auto& decl : sourceFile.getTopLevelDecls()) {
            printDecl(*decl);
        }
        indentLevel--;
        out << ")\n";
    }
}
