#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include <clang/AST/DeclBase.h>
#include <clang/Frontend/CompilerInstance.h>
#include <map>
#include <memory>
#include <set>
#include <yaml-cpp/yaml.h>

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
    void LoadFile(std::string filename);
    
    /**
     * Process the configuration settings against the current AST.
     */
    std::unique_ptr<CompiledConfiguration> Process(clang::CompilerInstance *ci) const;

    /**
     * Get the root node of the YAML configuration structure.
     */
    const YAML::Node& GetRoot() const;

private:
    Configuration();

protected:
    YAML::Node rootNode_;
};

class CompiledConfiguration
{
public:
    CompiledConfiguration(const CompiledConfiguration&) = delete;
    CompiledConfiguration &operator=(const CompiledConfiguration&) = delete;

    /**
     * Return list of namespace declarations that should be included.
     */
    const std::set<const clang::NamespaceDecl*>& GetNamespaces() const;

    /**
     * Get the YAML configuration associated with a specific declaration,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node& GetDeclaration(const clang::Decl *decl) const;

private:
    CompiledConfiguration();

protected:
    std::set<const clang::NamespaceDecl*> namespaces_;
    std::map<const clang::Decl*, YAML::Node> declarations_;
    static const YAML::Node emptyNode_;

    friend class Configuration;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__
