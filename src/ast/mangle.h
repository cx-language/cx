#pragma once

#include <string>

namespace llvm {

class StringRef;
template<typename T>
class ArrayRef;

} // namespace llvm

namespace delta {

struct Type;
class FunctionDecl;
class FunctionTemplate;
class TypeTemplate;
class TypeDecl;
class FieldDecl;
class EnumDecl;

std::string mangle(const FunctionDecl& decl);
std::string mangleFunctionDecl(Type receiver, llvm::StringRef name, llvm::ArrayRef<Type> genericArgs);
std::string mangle(const FunctionTemplate& decl);
std::string mangleFunctionTemplate(llvm::StringRef receiverType, llvm::StringRef functionName);
std::string mangle(const TypeDecl& decl);
std::string mangleTypeDecl(llvm::StringRef typeName, llvm::ArrayRef<Type> genericArgs);
std::string mangle(const FieldDecl& decl);
std::string mangle(const EnumDecl& decl);

} // namespace delta
