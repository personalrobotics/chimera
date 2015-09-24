#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include <clang/AST/DeclBase.h>
#include <clang/AST/Mangle.h>
#include <clang/Frontend/CompilerInstance.h>
#include <map>
#include <memory>
#include <set>
#include <yaml-cpp/yaml.h>

// Forward declare LLVM raw_ostream, as per:
// http://llvm.org/docs/CodingStandards.html#use-raw-ostream
namespace llvm
{
class raw_pwrite_stream;
}

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

    /**
     * Get the filename of the loaded YAML configuration file, if it exists.
     */
    const std::string& GetFilename() const;

private:
    Configuration();

protected:
    YAML::Node rootNode_;
    std::string rootFilename_;
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
     * Get a file pointer used for the output a given decl.
     *
     * This output path can be either `stdout`, a reference to a monolithic
     * `.cpp` file for all bindings, or an individual `.cpp` file created
     * according to the mangled name of the decl.
     *
     * The file pointer should be closed after the output has been written.
     */
    llvm::raw_pwrite_stream *GetOutputFile(const clang::Decl *decl) const;

private:
    CompiledConfiguration(clang::CompilerInstance *ci);

protected:
    static const YAML::Node emptyNode_;
    clang::CompilerInstance *ci_;
    std::map<const clang::Decl*, YAML::Node> declarations_;
    std::set<const clang::NamespaceDecl*> namespaces_;
    std::unique_ptr<clang::MangleContext> mangler_;

    friend class Configuration;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__
