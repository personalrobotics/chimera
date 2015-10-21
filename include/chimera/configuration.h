#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include "chimera/stream.h"

#include <clang/AST/DeclBase.h>
#include <clang/AST/Mangle.h>
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
    void LoadFile(const std::string &filename);

    /**
     * Set the desired output path to be prepended to every binding.
     * If unspecified, the default is the current working directory.
     */
    void SetOutputPath(const std::string &path);
    
    /**
     * Process the configuration settings against the current AST.
     */
    std::unique_ptr<CompiledConfiguration>
    Process(clang::CompilerInstance *ci, clang::StringRef file) const;

    /**
     * Get the root node of the YAML configuration structure.
     */
    const YAML::Node& GetRoot() const;

    /**
     * Get the filename of the loaded YAML configuration file, if it exists.
     */
    const std::string& GetFilename() const;

    /**
     * Get the desired output path for C++ bindings.
     */
    const std::string &GetOutputPath() const;

private:
    Configuration();

protected:
    YAML::Node rootNode_;
    std::string rootFilename_;
    std::string outputPath_;
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

    /**
     * Get the YAML configuration associated with a specific qualified type,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node& GetType(const clang::QualType type) const;

    /**
     * Get a file pointer used for the output a given decl.
     *
     * This output path can be either `stdout`, a reference to a monolithic
     * `.cpp` file for all bindings, or an individual `.cpp` file created
     * according to the mangled name of the decl.
     *
     * The file pointer should be closed after the output has been written.
     */
    std::unique_ptr<Stream>GetOutputFile(const clang::Decl *decl) const;

    /**
     * Dump any hard-coded overrides in place of this declaration, if they
     * are defined in the `content` (for strings) or `source` (for files)
     * fields of the YAML configuration for this declaration.
     *
     * Returns `true` if an override was found and dumped, `false` otherwise.
     */
    bool DumpOverride(const clang::Decl *decl, chimera::Stream &stream) const;

private:
    CompiledConfiguration(const Configuration &parent,
                          clang::CompilerInstance *ci,
                          clang::StringRef file);

protected:
    static const YAML::Node emptyNode_;
    const Configuration &parent_;
    clang::CompilerInstance *ci_;
    const clang::StringRef file_;
    std::vector<std::pair<const clang::QualType, YAML::Node>> types_;
    std::map<const clang::Decl*, YAML::Node> declarations_;
    std::set<const clang::NamespaceDecl*> namespaces_;
    std::unique_ptr<clang::MangleContext> mangler_;

    friend class Configuration;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__
