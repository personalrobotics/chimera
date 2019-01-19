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

#include <sstream>
#include <clang/AST/AST.h>
#include <clang/AST/Comment.h>
#include <mstch/mstch.hpp>

namespace chimera
{
namespace mstch
{

/**
 * Base mstch wrapper for Clang declarations.
 */
template <typename T>
class ClangWrapper : public ::mstch::object
{
    static_assert(std::is_base_of<clang::NamedDecl, T>::value,
                  "'T' must derive from clang::NamedDecl");

public:
    ClangWrapper(const ::chimera::CompiledConfiguration &config, const T *decl,
                 bool last = false)
      : config_(config)
      , decl_(decl)
      , decl_config_(config_.GetDeclaration(decl_))
      , last_(last)
    {
        // Add entries from the YAML configuration directly into the object.
        // This wraps each YAML node in a recursive conversion wrapper.
        for (YAML::const_iterator it = decl_config_.begin();
             it != decl_config_.end(); ++it)
        {
            const std::string name = it->first.as<std::string>();
            const YAML::Node &value = it->second;
            register_lambda(
                name, [value]() { return chimera::util::wrapYAMLNode(value); });
        }

        // Override certain entries with our clang-generated information.
        register_methods(
            this,
            {
                {"last", &ClangWrapper::last},
                {"name", &ClangWrapper::name},
                {"mangled_name", &ClangWrapper::mangledName},
                {"qualified_name", &ClangWrapper::qualifiedName},
                {"namespace_scope", &ClangWrapper::namespaceScope},
                {"class_scope", &ClangWrapper::classScope},
                {"scope",
                 &ClangWrapper::scope}, // namespace_scope + class_scope
                {"comment", &ClangWrapper::comment},
                {"comment?", &ClangWrapper::isNonFalse<ClangWrapper,
                                                       &ClangWrapper::comment>},
            });
    }

    ::mstch::node last()
    {
        return last_;
    }

    virtual ::mstch::node name()
    {
        if (const YAML::Node &node = decl_config_["name"])
        {
            // Convert a `null` to an empty string.
            // This helps users semantically mark names that should be
            // omitted in their configuration files, although we will
            // actually ignore any name that evaluates to an empty string.
            if (node.IsNull())
            {
                return std::string{""};
            }
            return node.as<std::string>();
        }

        return decl_->getNameAsString();
    }

    virtual ::mstch::node mangledName()
    {
        if (const YAML::Node &node = decl_config_["mangled_name"])
            return node.as<std::string>();

        return chimera::util::constructMangledName(decl_);
    }

    virtual ::mstch::node namespaceScope()
    {
        // Default is empty, overriden in subclasses.
        return std::string{""};
    }

    virtual ::mstch::node classScope()
    {
        // Default is empty, overriden in subclasses.
        return std::string{""};
    }

    /**
     * namespaceScope + classScope
     */
    virtual ::mstch::node scope()
    {
        // Default is empty, overriden in subclasses.
        return std::string{""};
    }

    virtual ::mstch::node qualifiedName()
    {
        if (const YAML::Node &node = decl_config_["qualified_name"])
            return node.as<std::string>();

        return decl_->getQualifiedNameAsString();
    }

    virtual ::mstch::node comment()
    {
        ::mstch::array comment_lines;
        const auto *comment = config_.GetContext().getCommentForDecl(
            decl_->getCanonicalDecl(), nullptr);

        // Ignore empty/missing comments.
        if (comment == nullptr)
            return comment_lines;

        // Aggregate comment blocks into one large string.
        for (auto comment_it = comment->child_begin();
             comment_it != comment->child_end(); ++comment_it)
        {
            // Look for block comment sections.
            const auto *commentSection = *comment_it;
            if (commentSection->getCommentKind()
                != clang::comments::BlockContentComment::ParagraphCommentKind)
                continue;

            // Combine the inline text of these sections together.
            for (auto text_it = commentSection->child_begin();
                 text_it != commentSection->child_end(); ++text_it)
            {
                if (clang::isa<clang::comments::TextComment>(*text_it))
                {
                    auto text
                        = clang::cast<clang::comments::TextComment>(*text_it);
                    comment_lines.push_back(text->getText().str());
                }
            }
        }

        return comment_lines;
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

    template <typename Derived, ::mstch::node (Derived::*Func)()>
    ::mstch::node isNonFalse()
    {
        ::mstch::map context{{"data", (static_cast<Derived *>(this)->*Func)()}};
        return ::mstch::render("{{^data}}NONE{{/data}}", context).empty();
    }
};

class CXXRecord : public ClangWrapper<clang::CXXRecordDecl>
{
public:
    CXXRecord(const ::chimera::CompiledConfiguration &config,
              const clang::CXXRecordDecl *decl,
              const std::set<const clang::CXXRecordDecl *> *available_decls
              = NULL);

    ::mstch::node bases();
    ::mstch::node type();
    ::mstch::node namespaceScope() override;
    ::mstch::node classScope() override;
    ::mstch::node scope() override;
    ::mstch::node isCopyable();

    ::mstch::node name() override;
    ::mstch::node qualifiedName() override;
    ::mstch::node heldType();

    ::mstch::node constructors();
    ::mstch::node methods();
    ::mstch::node staticMethods();

    ::mstch::node fields();
    ::mstch::node staticFields();

protected:
    const std::set<const clang::CXXRecordDecl *> *available_decls_;
};

class Enum : public ClangWrapper<clang::EnumDecl>
{
public:
    Enum(const ::chimera::CompiledConfiguration &config,
         const clang::EnumDecl *decl);

    ::mstch::node qualifiedName() override;
    ::mstch::node namespaceScope() override;
    ::mstch::node classScope() override;
    ::mstch::node scope() override;
    ::mstch::node type();
    ::mstch::node values();
};

class EnumConstant : public ClangWrapper<clang::EnumConstantDecl>
{
public:
    EnumConstant(const ::chimera::CompiledConfiguration &config,
                 const clang::EnumConstantDecl *decl,
                 const clang::EnumDecl *enum_decl);

    ::mstch::node qualifiedName() override;

private:
    const clang::EnumDecl *enum_decl_;
};

class Field : public ClangWrapper<clang::FieldDecl>
{
public:
    Field(const ::chimera::CompiledConfiguration &config,
          const clang::FieldDecl *decl, const clang::CXXRecordDecl *class_decl);

    ::mstch::node isAssignable();
    ::mstch::node isCopyable();
    ::mstch::node returnValuePolicy();
    ::mstch::node qualifiedName() override;

private:
    const clang::CXXRecordDecl *class_decl_;
};

// TODO: refactor Function to not need class_decl at all.
class Function : public ClangWrapper<clang::FunctionDecl>,
                 public std::enable_shared_from_this<Function>
{
public:
    Function(const ::chimera::CompiledConfiguration &config,
             const clang::FunctionDecl *decl,
             const clang::CXXRecordDecl *class_decl = NULL,
             const int argument_limit = -1);

    ::mstch::node type();
    ::mstch::node overloads();
    ::mstch::node params();
    ::mstch::node returnType();
    ::mstch::node returnValuePolicy();
    ::mstch::node isVoid();
    ::mstch::node namespaceScope() override;
    ::mstch::node classScope() override;
    ::mstch::node scope() override;
    ::mstch::node usesDefaults();
    ::mstch::node qualifiedName() override;
    ::mstch::node isTemplate();
    ::mstch::node call();
    ::mstch::node qualifiedCall();

private:
    const clang::CXXRecordDecl *class_decl_;
    const int argument_limit_;
};

class Method : public Function
{
public:
    Method(const ::chimera::CompiledConfiguration &config,
           const clang::CXXMethodDecl *decl,
           const clang::CXXRecordDecl *class_decl = NULL);

    ::mstch::node isConst();
    ::mstch::node isStatic();

private:
    const clang::CXXMethodDecl *method_decl_;
};

class Namespace : public ClangWrapper<clang::NamespaceDecl>
{
public:
    Namespace(const ::chimera::CompiledConfiguration &config,
              const clang::NamespaceDecl *decl);

    ::mstch::node scope();
};

class Parameter : public ClangWrapper<clang::ParmVarDecl>
{
public:
    Parameter(const ::chimera::CompiledConfiguration &config,
              const clang::ParmVarDecl *decl,
              const clang::FunctionDecl *method_decl,
              const clang::CXXRecordDecl *class_decl,
              const std::string default_name = "");

    ::mstch::node name() override;
    ::mstch::node type();

private:
    const clang::FunctionDecl *method_decl_;
    const clang::CXXRecordDecl *class_decl_;
    const std::string default_name_;
};

class Variable : public ClangWrapper<clang::VarDecl>
{
public:
    Variable(const ::chimera::CompiledConfiguration &config,
             const clang::VarDecl *decl,
             const clang::CXXRecordDecl *class_decl = NULL);

    ::mstch::node isAssignable();
    ::mstch::node qualifiedName() override;
    ::mstch::node namespaceScope() override;
    ::mstch::node classScope() override;
    ::mstch::node scope() override;

private:
    const clang::CXXRecordDecl *class_decl_;
};

} // namespace mstch
} // namespace chimera

#endif // __CHIMERA_MSTCH_H__
