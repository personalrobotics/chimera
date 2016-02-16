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
#include "chimera/util.h"

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
    ClangWrapper(const ::chimera::CompiledConfiguration &config,
                 const T *decl, bool last=false)
    : config_(config), decl_(decl)
    , decl_config_(config_.GetDeclaration(decl_))
    , last_(last)
    {
        register_methods(this, {
            {"config", &ClangWrapper::config},
            {"last", &ClangWrapper::last},
            {"name", &ClangWrapper::name},
            {"mangled_name", &ClangWrapper::mangledName},
            {"override", &ClangWrapper::override},
            {"qualified_name", &ClangWrapper::qualifiedName}
        });
    }

    ::mstch::node config()
    {
        ::mstch::map mstch_config;
        for(YAML::const_iterator it=decl_config_.begin(); it!=decl_config_.end(); ++it) {
            std::string value = config_.Lookup(it->second);
            if (!value.empty())
            {
                mstch_config[it->first.as<std::string>()] = value;
            }
        }
        return mstch_config;
    }

    ::mstch::node last()
    {
        return last_;
    }

    ::mstch::node name()
    {
        if (const YAML::Node &node = decl_config_["name"])
            return node.as<std::string>();

        return decl_->getNameAsString();
    }

    ::mstch::node mangledName()
    {
        if (const YAML::Node &node = decl_config_["mangled_name"])
            return node.as<std::string>();

        return chimera::util::constructMangledName(
            config_.GetContext(), decl_);
    }

    ::mstch::node override()
    {
        // TODO: implement this method.
        return "";
    }

    ::mstch::node qualifiedName()
    {
        if (const YAML::Node &node = decl_config_["qualified_name"])
            return node.as<std::string>();

        return decl_->getQualifiedNameAsString();
    }

    void setLast(bool is_last)
    {
        last_ = is_last;
    }

protected:
    const ::chimera::CompiledConfiguration &config_;
    const T *decl_;
    const YAML::Node &decl_config_;
    bool last_;
};

class CXXRecord: public ClangWrapper<clang::CXXRecordDecl>
{
public:
    CXXRecord(const ::chimera::CompiledConfiguration &config,
              const clang::CXXRecordDecl *decl,
              const std::set<const clang::CXXRecordDecl*> *available_decls = NULL);

    ::mstch::node bases();
    ::mstch::node type();
    ::mstch::node isCopyable();

    ::mstch::node bindingName();
    ::mstch::node uniquishName();
    ::mstch::node mangledName();
    
    ::mstch::node constructors();
    ::mstch::node methods();
    
    ::mstch::node fields();
    ::mstch::node staticFields();

protected:
    const std::set<const clang::CXXRecordDecl *> *available_decls_;
};

class Enum: public ClangWrapper<clang::EnumDecl>
{
public:
    Enum(const ::chimera::CompiledConfiguration &config,
         const clang::EnumDecl *decl);

    ::mstch::node type();
    ::mstch::node values();
};

using EnumConstant = ClangWrapper<clang::EnumConstantDecl>;

class Field: public ClangWrapper<clang::FieldDecl>
{
public:
    Field(const ::chimera::CompiledConfiguration &config,
          const clang::FieldDecl *decl,
          const clang::CXXRecordDecl *class_decl);

    ::mstch::node isAssignable();
    ::mstch::node isCopyable();
    ::mstch::node returnValuePolicy();
    ::mstch::node qualifiedName();

private:
    const clang::CXXRecordDecl *class_decl_;
};

// TODO: refactor Function to not need class_decl at all.
class Function: public ClangWrapper<clang::FunctionDecl>
{
public:
    Function(const ::chimera::CompiledConfiguration &config,
             const clang::FunctionDecl *decl,
             const clang::CXXRecordDecl *class_decl=NULL);

    ::mstch::node type();
    ::mstch::node params();
    ::mstch::node returnValuePolicy();
    ::mstch::node qualifiedName();

private:
    const clang::CXXRecordDecl *class_decl_;
};

class Method: public Function
{
public:
    Method(const ::chimera::CompiledConfiguration &config,
           const clang::CXXMethodDecl *decl,
           const clang::CXXRecordDecl *class_decl=NULL);

    ::mstch::node isStatic();

private:
    const clang::CXXMethodDecl *method_decl_;
};

class Parameter: public ClangWrapper<clang::ParmVarDecl>
{
public:
    Parameter(const ::chimera::CompiledConfiguration &config,
              const clang::ParmVarDecl *decl,
              const clang::FunctionDecl *method_decl,
              const clang::CXXRecordDecl *class_decl,
              bool use_default=true);

    ::mstch::node name();
    ::mstch::node type();
    ::mstch::node value();

private:
    const clang::CXXRecordDecl *class_decl_;
    const clang::FunctionDecl *method_decl_;
    const bool use_default_;
};

class Variable: public ClangWrapper<clang::VarDecl>
{
public:
    Variable(const ::chimera::CompiledConfiguration &config,
             const clang::VarDecl *decl,
             const clang::CXXRecordDecl *class_decl=NULL);

    ::mstch::node isAssignable();
    ::mstch::node qualifiedName();

private:
    const clang::CXXRecordDecl *class_decl_;
};

} // namespace mstch
} // namespace chimera


#endif // __CHIMERA_MSTCH_H__
