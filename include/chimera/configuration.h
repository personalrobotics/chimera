#ifndef __CHIMERA_CONFIGURATION_H__
#define __CHIMERA_CONFIGURATION_H__

#include <clang/AST/DeclBase.h>
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
    const YAML::Node& GetRoot() const;

    /**
     * Get the YAML configuration associated with a specific declaration,
     * or return an empty YAML node if no configuration was found.
     */
    const YAML::Node& GetSignature(const clang::Decl& declaration) const;

private:
    Configuration();

protected:
    YAML::Node rootNode_;
    YAML::Node emptyNode_;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__