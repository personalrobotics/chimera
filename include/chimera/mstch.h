/**
 * Template wrapper functions for Clang declarations.
 *
 * These wrapper classes take in a Clang declaration and provide helper methods
 * that can be used within an `mstch` template to generate bindings.  Each
 * helper method returns a `mstch::node`, which is a Boost::Variant type that
 * can be a primitive type, std::string, `mstch::object` or `mstch::array`.
 */
#ifndef __CHIMERA_MSTCH_H__
#define __CHIMERA_MSTCH_H__

#include "chimera/configuration.h"
#include <mstch/mstch.hpp>
#include <clang/AST/ASTContext.h>

namespace chimera
{
namespace mstch
{

class ClangWrapper: public ::mstch::object
{
public:
    ClangWrapper(const clang::NamedDecl *decl,
                 const ::chimera::CompiledConfiguration &config);

};

class CXXRecord: public ::mstch::object
{
public:
    CXXRecord(const clang::CXXRecordDecl *decl,
              const ::chimera::CompiledConfiguration &config);

    ::mstch::node bases();
    ::mstch::node type();
    ::mstch::node isCopyable();

    ::mstch::node name();
    ::mstch::node uniquishName();
    ::mstch::node mangledName();
    
    ::mstch::node constructors();
    
    ::mstch::node methods();
    ::mstch::node staticMethods();
    
    ::mstch::node fields();
    ::mstch::node staticFields();

private:
    const clang::CXXRecordDecl *decl_;
    const ::chimera::CompiledConfiguration &config_;
};

class Enum: public ::mstch::object
{
public:
    Enum(const clang::EnumDecl *decl,
         const ::chimera::CompiledConfiguration &config);

    ::mstch::node name();
    ::mstch::node uniquishName();
    ::mstch::node mangledName();
    
    ::mstch::node values();

private:
    const clang::EnumDecl *decl_;
    const ::chimera::CompiledConfiguration &config_;
};

class Function: public ::mstch::object
{
public:
    Function(const clang::FunctionDecl *decl,
             const ::chimera::CompiledConfiguration &config);

    ::mstch::node name();
    ::mstch::node uniquishName();
    ::mstch::node mangledName();

    ::mstch::node type();
    ::mstch::node params();
    ::mstch::node returnValuePolicy();

private:
    const clang::FunctionDecl *decl_;
    const ::chimera::CompiledConfiguration &config_;
};

class GlobalVar: public ::mstch::object
{
public:
    GlobalVar(const clang::VarDecl *decl,
              const ::chimera::CompiledConfiguration &config);

    ::mstch::node name();
    ::mstch::node uniquishName();
    ::mstch::node mangledName();

    ::mstch::node type();
    ::mstch::node params();
    ::mstch::node returnValuePolicy();

private:
    const clang::VarDecl *decl_;
    const ::chimera::CompiledConfiguration &config_;
};

} // namespace mstch
} // namespace chimera


#endif // __CHIMERA_MSTCH_H__
