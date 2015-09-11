#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <yaml-cpp/yaml.h>

namespace chimera
{

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
     * Get the root node of the YAML configuration structure.
     */
    YAML::Node GetRoot();

    /**
     * Get the YAML configuration associated with a specific type declaration,
     * or return an empty YAML node if no configuration was found.
     */
    YAML::Node GetSignature(std::string signature);

private:
    Configuration();

protected:
    YAML::Node rootNode_;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__