#include "ast-print.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"

namespace cx {

int indent = 0;

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Decl& decl);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Stmt& stmt);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Expr& expr);

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, llvm::ArrayRef<Stmt*> stmts) {
    indent++;
    for (auto* stmt : stmts) {
        stream << std::string(indent * 4, ' ') << *stmt << '\n';
    }
    indent--;
    return stream;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, llvm::ArrayRef<NamedValue> namedValues) {
    indent++;
    for (auto& namedValue : namedValues) {
        stream << std::string(indent * 4, ' ');
        if (!namedValue.getName().empty()) {
            stream << namedValue.getName() << " ";
        }
        stream << *namedValue.getValue() << '\n';
    }
    indent--;
    return stream;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::GenericParamDecl: {
            //            auto& genericParamDecl = llvm::cast<GenericParamDecl>(decl);
            stream << "GenericParamDecl";
            break;
        }
        case DeclKind::FunctionDecl: {
            auto& functionDecl = llvm::cast<FunctionDecl>(decl);
            stream << "FunctionDecl " << functionDecl.getQualifiedName() << '\n';
            if (functionDecl.hasBody()) {
                stream << functionDecl.getBody();
            }
            break;
        }
        case DeclKind::MethodDecl: {
            //            auto& methodDecl = llvm::cast<MethodDecl>(decl);
            stream << "MethodDecl";
            break;
        }
        case DeclKind::ConstructorDecl: {
            //            auto& constructorDecl = llvm::cast<ConstructorDecl>(decl);
            stream << "ConstructorDecl";
            break;
        }
        case DeclKind::DestructorDecl: {
            //            auto& destructorDecl = llvm::cast<DestructorDecl>(decl);
            stream << "DestructorDecl";
            break;
        }
        case DeclKind::FunctionTemplate: {
            //            auto& functionTemplate = llvm::cast<FunctionTemplate>(decl);
            stream << "FunctionTemplate";
            break;
        }
        case DeclKind::TypeDecl: {
            //            auto& typeDecl = llvm::cast<TypeDecl>(decl);
            stream << "TypeDecl";
            break;
        }
        case DeclKind::TypeTemplate: {
            //            auto& typeTemplate = llvm::cast<TypeTemplate>(decl);
            stream << "TypeTemplate";
            break;
        }
        case DeclKind::EnumDecl: {
            //            auto& enumDecl = llvm::cast<EnumDecl>(decl);
            stream << "EnumDecl";
            break;
        }
        case DeclKind::EnumCase: {
            //            auto& enumCase = llvm::cast<EnumCase>(decl);
            stream << "EnumCase";
            break;
        }
        case DeclKind::VarDecl: {
            auto& varDecl = llvm::cast<VarDecl>(decl);
            stream << "VarDecl " << varDecl.getName() << " ";
            if (varDecl.getInitializer()) {
                stream << *varDecl.getInitializer();
            }
            break;
        }
        case DeclKind::FieldDecl: {
            //            auto& fieldDecl = llvm::cast<FieldDecl>(decl);
            stream << "FieldDecl";
            break;
        }
        case DeclKind::ParamDecl: {
            //            auto& paramDecl = llvm::cast<ParamDecl>(decl);
            stream << "ParamDecl";
            break;
        }
        case DeclKind::ImportDecl: {
            //            auto& importDecl = llvm::cast<ImportDecl>(decl);
            stream << "ImportDecl";
            break;
        }
    }

    return stream;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt: {
            //            auto& returnStmt = llvm::cast<ReturnStmt>(stmt);
            stream << "ReturnStmt";
            break;
        }
        case StmtKind::VarStmt: {
            auto& varStmt = llvm::cast<VarStmt>(stmt);
            stream << "VarStmt " << varStmt.getDecl();
            break;
        }
        case StmtKind::ExprStmt: {
            auto& exprStmt = llvm::cast<ExprStmt>(stmt);
            stream << "ExprStmt " << exprStmt.getExpr();
            break;
        }
        case StmtKind::DeferStmt: {
            //            auto& deferStmt = llvm::cast<DeferStmt>(stmt);
            stream << "DeferStmt";
            break;
        }
        case StmtKind::IfStmt: {
            auto& ifStmt = llvm::cast<IfStmt>(stmt);
            stream << "IfStmt " << ifStmt.getCondition() << "\n" << ifStmt.getThenBody();
            break;
        }
        case StmtKind::SwitchStmt: {
            //            auto& switchStmt = llvm::cast<SwitchStmt>(stmt);
            stream << "SwitchStmt";
            break;
        }
        case StmtKind::WhileStmt: {
            //            auto& whileStmt = llvm::cast<WhileStmt>(stmt);
            stream << "WhileStmt";
            break;
        }
        case StmtKind::ForStmt: {
            //            auto& forStmt = llvm::cast<ForStmt>(stmt);
            stream << "ForStmt";
            break;
        }
        case StmtKind::ForEachStmt: {
            //            auto& forEachStmt = llvm::cast<ForEachStmt>(stmt);
            stream << "ForEachStmt";
            break;
        }
        case StmtKind::BreakStmt: {
            //            auto& breakStmt = llvm::cast<BreakStmt>(stmt);
            stream << "BreakStmt";
            break;
        }
        case StmtKind::ContinueStmt: {
            //            auto& continueStmt = llvm::cast<ContinueStmt>(stmt);
            stream << "ContinueStmt";
            break;
        }
        case StmtKind::CompoundStmt: {
            //            auto& compoundStmt = llvm::cast<CompoundStmt>(stmt);
            stream << "CompoundStmt";
            break;
        }
    }

    return stream;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VarExpr: {
            auto& varExpr = llvm::cast<VarExpr>(expr);
            stream << "VarExpr " << varExpr.getIdentifier() << " type=" << varExpr.getTypeOrNull();
            break;
        }
        case ExprKind::StringLiteralExpr: {
            //            auto& stringLiteralExpr = llvm::cast<StringLiteralExpr>(expr);
            stream << "StringLiteralExpr";
            break;
        }
        case ExprKind::CharacterLiteralExpr: {
            //            auto& characterLiteralExpr = llvm::cast<CharacterLiteralExpr>(expr);
            stream << "CharacterLiteralExpr";
            break;
        }
        case ExprKind::IntLiteralExpr: {
            //            auto& intLiteralExpr = llvm::cast<IntLiteralExpr>(expr);
            stream << "IntLiteralExpr";
            break;
        }
        case ExprKind::FloatLiteralExpr: {
            //            auto& floatLiteralExpr = llvm::cast<FloatLiteralExpr>(expr);
            stream << "FloatLiteralExpr";
            break;
        }
        case ExprKind::BoolLiteralExpr: {
            //            auto& boolLiteralExpr = llvm::cast<BoolLiteralExpr>(expr);
            stream << "BoolLiteralExpr";
            break;
        }
        case ExprKind::NullLiteralExpr: {
            //            auto& nullLiteralExpr = llvm::cast<NullLiteralExpr>(expr);
            stream << "NullLiteralExpr";
            break;
        }
        case ExprKind::UndefinedLiteralExpr: {
            //            auto& undefinedLiteralExpr = llvm::cast<UndefinedLiteralExpr>(expr);
            stream << "UndefinedLiteralExpr";
            break;
        }
        case ExprKind::ArrayLiteralExpr: {
            //            auto& arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(expr);
            stream << "ArrayLiteralExpr";
            break;
        }
        case ExprKind::TupleExpr: {
            //            auto& tupleExpr = llvm::cast<TupleExpr>(expr);
            stream << "TupleExpr";
            break;
        }
        case ExprKind::UnaryExpr: {
            //            auto& unaryExpr = llvm::cast<UnaryExpr>(expr);
            stream << "UnaryExpr";
            break;
        }
        case ExprKind::BinaryExpr: {
            //            auto& binaryExpr = llvm::cast<BinaryExpr>(expr);
            stream << "BinaryExpr";
            break;
        }
        case ExprKind::CallExpr: {
            auto& callExpr = llvm::cast<CallExpr>(expr);
            stream << "CallExpr " << callExpr.getCallee() << "\n" << callExpr.getArgs();
            break;
        }
        case ExprKind::SizeofExpr: {
            //            auto& sizeofExpr = llvm::cast<SizeofExpr>(expr);
            stream << "SizeofExpr";
            break;
        }
        case ExprKind::AddressofExpr: {
            //            auto& addressofExpr = llvm::cast<AddressofExpr>(expr);
            stream << "AddressofExpr";
            break;
        }
        case ExprKind::MemberExpr: {
            //            auto& memberExpr = llvm::cast<MemberExpr>(expr);
            stream << "MemberExpr";
            break;
        }
        case ExprKind::IndexExpr: {
            //            auto& indexExpr = llvm::cast<IndexExpr>(expr);
            stream << "IndexExpr";
            break;
        }
        case ExprKind::IndexAssignmentExpr: {
            //            auto& indexAssignmentExpr = llvm::cast<IndexAssignmentExpr>(expr);
            stream << "IndexAssignmentExpr";
            break;
        }
        case ExprKind::UnwrapExpr: {
            //            auto& unwrapExpr = llvm::cast<UnwrapExpr>(expr);
            stream << "UnwrapExpr";
            break;
        }
        case ExprKind::LambdaExpr: {
            //            auto& lambdaExpr = llvm::cast<LambdaExpr>(expr);
            stream << "LambdaExpr";
            break;
        }
        case ExprKind::IfExpr: {
            //            auto& ifExpr = llvm::cast<IfExpr>(expr);
            stream << "IfExpr";
            break;
        }
        case ExprKind::ImplicitCastExpr: {
            auto& implicitCastExpr = llvm::cast<ImplicitCastExpr>(expr);
            stream << "ImplicitCastExpr " << implicitCastExpr.getImplicitCastKind() << " " << *implicitCastExpr.getOperand();
            break;
        }
        case ExprKind::VarDeclExpr: {
            //            auto& varDeclExpr = llvm::cast<VarDeclExpr>(expr);
            stream << "VarDeclExpr";
            break;
        }
    }

    return stream;
}

} // namespace cx
