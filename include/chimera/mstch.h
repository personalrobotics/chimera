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

#include <clang/AST/AST.h>
#include <mstch/mstch.hpp>

namespace chimera
{
namespace mstch
{

template<typename T>
class ClangWrapper: public ::mstch::object
{
static_assert(std::is_base_of<clang::NamedDecl, T>::value,
              "'T' must derive from clang::NamedDecl");
public:
    ClangWrapper(const T *decl,
                 const ::chimera::CompiledConfiguration &config)
    : decl_(decl), config_(config)
    , decl_config_(config_.GetDeclaration(decl_))
    {
        register_methods(this, {
            {"name", &ClangWrapper::name}
        });
    }

    ::mstch::node name()
    {
        if (const YAML::Node &node = decl_config_["name"])
            return node.as<std::string>();

        return decl_->getNameAsString();
    }

protected:
    const T *decl_;
    const ::chimera::CompiledConfiguration &config_;
    const YAML::Node &decl_config_;
};

class CXXRecord: public ClangWrapper<clang::CXXRecordDecl>
{
public:
    CXXRecord(const clang::CXXRecordDecl *decl,
              const ::chimera::CompiledConfiguration &config);

    ::mstch::node bases();
    ::mstch::node type();
    ::mstch::node isCopyable();

    ::mstch::node bindingName();
    ::mstch::node uniquishName();
    ::mstch::node mangledName();
    
    ::mstch::node constructors();
    
    ::mstch::node methods();
    ::mstch::node staticMethods();
    
    ::mstch::node fields();
    ::mstch::node staticFields();
};

class Enum: public ClangWrapper<clang::EnumDecl>
{
public:
    Enum(const clang::EnumDecl *decl,
         const ::chimera::CompiledConfiguration &config);

    ::mstch::node type();
    ::mstch::node values();
};

class Function: public ClangWrapper<clang::FunctionDecl>
{
public:
    Function(const clang::FunctionDecl *decl,
             const ::chimera::CompiledConfiguration &config);

    ::mstch::node uniquishName();
    ::mstch::node mangledName();

    ::mstch::node type();
    ::mstch::node params();
    ::mstch::node returnValuePolicy();
};

class GlobalVar: public ClangWrapper<clang::VarDecl>
{
public:
    GlobalVar(const clang::VarDecl *decl,
              const ::chimera::CompiledConfiguration &config);

    ::mstch::node uniquishName();
    ::mstch::node mangledName();

    ::mstch::node type();
    ::mstch::node params();
    ::mstch::node returnValuePolicy();
};

} // namespace mstch
} // namespace chimera


#endif // __CHIMERA_MSTCH_H__
