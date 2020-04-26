#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include "chimera/binding.h"

#include <map>
#include <memory>
#include <set>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Mangle.h>
#include <clang/Frontend/CompilerInstance.h>
#include <mstch/mstch.hpp>
#include <yaml-cpp/yaml.h>

namespace chimera
{
namespace mstch
{

class CXXRecord;
class Enum;
class Function;
class Variable;
class Namespace;
class Typedef;

} // namespace mstch
} // namespace chimera

namespace chimera
{

class CompiledConfiguration;

class Configuration
{
public:
    Configuration();
    Configuration(const Configuration &) = delete;
    Configuration &operator=(const Configuration &) = delete;

    /**
     * Loads the specified file to use as the YAML configuration.
     */
    void LoadFile(const std::string &filename);

    /**
     * Sets the desired binding definition by name.
     * If unspecified, the default is "boost_python".
     */
    void SetBindingName(const std::string &name);

    /**
     * Sets the desired output path to be prepended to every binding.
     * If unspecified, the default is the current working directory.
     */
    void SetOutputPath(const std::string &path);

    /**
     * Sets the desired output python module for the top-level binding.
     * If unspecified, the default is "chimera_binding".
     */
    void SetOutputModuleName(const std::string &moduleName);

    /**
     * Appends an enclosing namespace that should be generated as part
     * of the top-level binding.
     */
    void AddInputNamespaceName(const std::string &namespaceName);

    /**
     * Appends the path to a source file that will be processed as part
     * of the binding generation.
     */
    void AddSourcePath(const std::string &sourcePath);

    /**
     * Sets whether to treat unresolvable configuration as errors.
     */
    void SetStrict(bool val);

    /**
     * Processes the configuration settings against the current AST.
     */
    std::unique_ptr<CompiledConfiguration> Process(
        clang::CompilerInstance *ci) const;

    /**
     * Gets the root node of the YAML configuration structure.
     */
    const YAML::Node &GetRoot() const;

    /**
     * Gets the filename of the loaded YAML configuration file, if it exists.
     */
    const std::string &GetConfigFilename() const;

    /**
     * Gets the desired output path for bindings.
     */
    const std::string &GetOutputPath() const;

    /**
     * Gets the desired output python module name for top-level binding.
     */
    const std::string &GetOutputModuleName() const;

protected:
    YAML::Node configNode_;
    std::string bindingName_;
    std::string configFilename_;
    std::string outputPath_;
    std::string outputModuleName_;
    std::vector<std::string> inputNamespaceNames_;
    std::vector<std::string> inputSourcePaths_;
    bool strict_;

    friend class CompiledConfiguration;
};

class CompiledConfiguration
{
public:
    enum class StaticMethodNamePolicy
    {
        NO_CHANGE,
        // Cast all the letters to upper case
        TO_UPPER,
        // Cast all the letters to lower case
        TO_LOWER,
        // Cast the first letter to upper case and cast all the letters followed
        // by underscore to upper cases removing the underscores
        TO_PASCAL,
        // Cast the first letter to lower case and cast all the letters followed
        // by underscore to upper cases removing the underscores
        TO_CAMEL,
        // TODO: Add more policies such as prefix and suffix
    };

    virtual ~CompiledConfiguration() = default;
    CompiledConfiguration(const CompiledConfiguration &) = delete;
    CompiledConfiguration &operator=(const CompiledConfiguration &) = delete;

    /**
     * Returns whether to treat unresolvable configuration as errors.
     */
    bool GetStrict() const;

    /**
     * Returns policy for the case that a static method has a same name one of
     * the instance method names in the same class.
     */
    StaticMethodNamePolicy GetStaticMethodNamePolicy() const;

    /**
     * Adds a namespace to an ordered set of traversed namespaces.
     * This set can later be rendered in a template.
     */
    void AddTraversedNamespace(const clang::NamespaceDecl *decl);

    /**
     * Returns list of namespace declarations that should be included.
     *
     * Note: these declarations will be lexically ordered.  Normally this
     * is desirable since parent namespaces will naturally be lexically
     * ordered before their children.
     */
    const std::set<const clang::NamespaceDecl *> &GetNamespacesIncluded() const;

    /**
     * Returns list of namespace declarations that should be skipped.
     */
    const std::set<const clang::NamespaceDecl *> &GetNamespacesSuppressed()
        const;

    /**
     * Gets the YAML configuration associated with a specific declaration,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node &GetDeclaration(const clang::Decl *decl) const;

    /**
     * Gets the YAML configuration associated with a specific qualified type,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node &GetType(const clang::QualType type) const;

    /**
     * Gets the compiler instance used by this configuration.
     */
    clang::CompilerInstance *GetCompilerInstance() const;

    /**
     * Gets the AST context used by this configuration.
     */
    clang::ASTContext &GetContext() const;

    /**
     * Gets the binding name of this configuration.
     *
     * The binding name is one of the following sources in order of priority:
     *   1) CLI '--binding' setting
     *   2) YAML configuration setting
     *   3) chimera::binding::DEFAULT_NAME
     */
    const std::string &GetBindingName() const;

    /**
     * Return if a declaration is enclosed by one of the configured namespaces.
     */
    bool IsEnclosed(const clang::Decl *decl) const;

    /**
     * Return if a declaration should not be generated.
     *
     * This can happen because it is marked as suppressed in the configuration,
     * or because it depends on a type that is marked as suppressed.
     */
    bool IsSuppressed(const clang::Decl *decl) const;

    /**
     * Return if a type should not be generated.
     *
     * This can happen because it is marked as suppressed in the configuration.
     */
    bool IsSuppressed(const clang::QualType type) const;

    /**
     * Checks if a particular node is a string or refers to a file to load.
     * This is useful for resolving entries that might be pulled in from files.
     *
     * Files are represented by string scalars that have a type tag of "!file".
     */
    std::string Lookup(const YAML::Node &node) const;

    /**
     * Renders a particular mstch template based on some declaration.
     * This context must contain a "mangled_name" from which to create the
     * filename.
     */
    bool Render(const std::shared_ptr<chimera::mstch::CXXRecord> context);
    bool Render(const std::shared_ptr<chimera::mstch::Enum> context);
    bool Render(const std::shared_ptr<chimera::mstch::Function> context);
    bool Render(const std::shared_ptr<chimera::mstch::Variable> context);
    bool Render(const std::shared_ptr<chimera::mstch::Typedef> context);

    /**
     * Renders the top-level mstch template. The rendered filename is specified
     * by Configuration::SetOutputModuleName().
     */
    void Render();

private:
    CompiledConfiguration(const Configuration &parent,
                          clang::CompilerInstance *ci);

    bool Render(const std::string &key, const std::string &header_view,
                const std::string &source_view,
                const std::shared_ptr<::mstch::object> &template_context);
    bool Render(const std::string &mangled_name, const std::string &view,
                const std::string &extension,
                const std::shared_ptr<::mstch::object> &context,
                const ::mstch::map &full_context);
    void SetBindingDefinitions(const std::string &key, std::string &header_def,
                               std::string &source_def);

protected:
    static const YAML::Node emptyNode_;
    const Configuration &parent_;
    const YAML::Node configNode_;
    const YAML::Node bindingNode_;
    std::string binding_name_;
    chimera::binding::Definition bindingDefinition_;
    clang::CompilerInstance *ci_;
    std::vector<std::pair<const clang::QualType, YAML::Node>> types_;
    std::map<const clang::Decl *, YAML::Node> declarations_;
    std::set<const clang::NamespaceDecl *> namespacesIncluded_;
    std::set<const clang::NamespaceDecl *> namespacesSuppressed_;

    std::vector<std::string> binding_names_;
    std::vector<std::shared_ptr<chimera::mstch::Namespace>> binding_namespaces_;
    std::set<const clang::NamespaceDecl *> binding_namespace_decls_;

    bool strict_;
    StaticMethodNamePolicy static_method_name_policy_;

    friend class Configuration;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__
