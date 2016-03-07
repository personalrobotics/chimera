#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include <clang/AST/DeclBase.h>
#include <clang/AST/Mangle.h>
#include <clang/Frontend/CompilerInstance.h>
#include <map>
#include <memory>
#include <mstch/mstch.hpp>
#include <set>
#include <yaml-cpp/yaml.h>

namespace chimera
{
namespace mstch
{

class CXXRecord;
class Enum;
class Function;
class Variable;

} // mstch
} // chimera


namespace chimera
{

class CompiledConfiguration;

class Configuration
{
public:
    Configuration(const Configuration&) = delete;
    Configuration &operator=(const Configuration&) = delete;

    /**
     * Get the chimera configuration singleton for this process.
     */
    static Configuration& GetInstance();

    /**
     * Load the specified file to use as the YAML configuration.
     */
    void LoadFile(const std::string &filename);

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
     * Process the configuration settings against the current AST.
     */
    std::unique_ptr<CompiledConfiguration>
    Process(clang::CompilerInstance *ci) const;

    /**
     * Get the root node of the YAML configuration structure.
     */
    const YAML::Node& GetRoot() const;

    /**
     * Get the filename of the loaded YAML configuration file, if it exists.
     */
    const std::string& GetConfigFilename() const;

    /**
     * Get the desired output path for bindings.
     */
    const std::string &GetOutputPath() const;

    /**
     * Get the desired output python module name for top-level binding.
     */
    const std::string& GetOutputModuleName() const;

private:
    Configuration();

protected:
    YAML::Node configNode_;
    std::string configFilename_;
    std::string outputPath_;
    std::string outputModuleName_;
};

class CompiledConfiguration
{
public:
    virtual ~CompiledConfiguration();
    CompiledConfiguration(const CompiledConfiguration&) = delete;
    CompiledConfiguration &operator=(const CompiledConfiguration&) = delete;

    /**
     * Adds a namespace to an ordered set of traversed namespaces.
     * This set can later be rendered in a template.
     */
    void AddTraversedNamespace(const clang::NamespaceDecl* decl);

    /**
     * Return list of namespace declarations that should be included.
     *
     * Note: these declarations will be lexically ordered.  Normally this
     * is desirable since parent namespaces will naturally be lexically
     * ordered before their children.
     */
    const std::set<const clang::NamespaceDecl*>& GetNamespaces() const;

    /**
     * Get the YAML configuration associated with a specific declaration,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node& GetDeclaration(const clang::Decl *decl) const;

    /**
     * Get the YAML configuration associated with a specific qualified type,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node& GetType(const clang::QualType type) const;

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
     * Checks if a particular node is a string or contains a "source" entry to load.
     * This is useful for resolving entries that might be pulled in from files.
     */
    std::string Lookup(const YAML::Node &node) const;

    /**
     * Render a particular mstch template based on some declaration.
     * This context must contain a "mangled_name" from which to create the filename.
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
    clang::CompilerInstance *ci_;
    std::vector<std::pair<const clang::QualType, YAML::Node>> types_;
    std::map<const clang::Decl*, YAML::Node> declarations_;
    std::set<const clang::NamespaceDecl*> namespaces_;

    std::vector<std::string> binding_names_;
    std::set<const clang::NamespaceDecl*> binding_namespaces_;

    friend class Configuration;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__
