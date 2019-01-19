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

} // namespace mstch
} // namespace chimera

namespace chimera
{

class CompiledConfiguration;

class Configuration
{
public:
    Configuration(const Configuration &) = delete;
    Configuration &operator=(const Configuration &) = delete;

    /**
     * Get the chimera configuration singleton for this process.
     */
    static Configuration &GetInstance();

    /**
     * Load the specified file to use as the YAML configuration.
     */
    void LoadFile(const std::string &filename);

    /**
     * Set the desired binding definition by name.
     * If unspecified, the default is "boost_python".
     */
    void SetBindingName(const std::string &name);

    /**
     * Set the desired output path to be prepended to every binding.
     * If unspecified, the default is the current working directory.
     */
    void SetOutputPath(const std::string &path);

    /**
     * Set the desired output python module for the top-level binding.
     * If unspecified, the default is "chimera_binding".
     */
    void SetOutputModuleName(const std::string &moduleName);

    /**
     * Append an enclosing namespace that should be generated as part
     * of the top-level binding.
     */
    void AddInputNamespaceName(const std::string &namespaceName);

    /**
     * Append the path to a source file that will be processed as part
     * of the binding generation.
     */
    void AddSourcePath(const std::string &sourcePath);

    /**
     * Process the configuration settings against the current AST.
     */
    std::unique_ptr<CompiledConfiguration> Process(
        clang::CompilerInstance *ci) const;

    /**
     * Get the root node of the YAML configuration structure.
     */
    const YAML::Node &GetRoot() const;

    /**
     * Get the filename of the loaded YAML configuration file, if it exists.
     */
    const std::string &GetConfigFilename() const;

    /**
     * Get the desired output path for bindings.
     */
    const std::string &GetOutputPath() const;

    /**
     * Get the desired output python module name for top-level binding.
     */
    const std::string &GetOutputModuleName() const;

private:
    Configuration();

protected:
    YAML::Node configNode_;
    std::string bindingName_;
    std::string configFilename_;
    std::string outputPath_;
    std::string outputModuleName_;
    std::vector<std::string> inputNamespaceNames_;
    std::vector<std::string> inputSourcePaths_;

    friend class CompiledConfiguration;
};

class CompiledConfiguration
{
public:
    virtual ~CompiledConfiguration();
    CompiledConfiguration(const CompiledConfiguration &) = delete;
    CompiledConfiguration &operator=(const CompiledConfiguration &) = delete;

    /**
     * Adds a namespace to an ordered set of traversed namespaces.
     * This set can later be rendered in a template.
     */
    void AddTraversedNamespace(const clang::NamespaceDecl *decl);

    /**
     * Return list of namespace declarations that should be included.
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
     * Get the YAML configuration associated with a specific declaration,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node &GetDeclaration(const clang::Decl *decl) const;

    /**
     * Get the YAML configuration associated with a specific qualified type,
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
     * Render a particular mstch template based on some declaration.
     * This context must contain a "mangled_name" from which to create the
     * filename.
     */
    bool Render(const std::shared_ptr<chimera::mstch::CXXRecord> context);
    bool Render(const std::shared_ptr<chimera::mstch::Enum> context);
    bool Render(const std::shared_ptr<chimera::mstch::Function> context);
    bool Render(const std::shared_ptr<chimera::mstch::Variable> context);

private:
    CompiledConfiguration(const Configuration &parent,
                          clang::CompilerInstance *ci);

    bool Render(std::string view, std::string key,
                const std::shared_ptr<::mstch::object> &template_context);

protected:
    static const YAML::Node emptyNode_;
    const Configuration &parent_;
    const YAML::Node configNode_;
    const YAML::Node bindingNode_;
    chimera::binding::Definition bindingDefinition_;
    clang::CompilerInstance *ci_;
    std::vector<std::pair<const clang::QualType, YAML::Node>> types_;
    std::map<const clang::Decl *, YAML::Node> declarations_;
    std::set<const clang::NamespaceDecl *> namespacesIncluded_;
    std::set<const clang::NamespaceDecl *> namespacesSuppressed_;

    std::vector<std::string> binding_names_;
    std::vector<std::shared_ptr<chimera::mstch::Namespace>> binding_namespaces_;
    std::set<const clang::NamespaceDecl *> binding_namespace_decls_;

    friend class Configuration;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__
