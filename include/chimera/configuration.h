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
    static Configuration& GetInstance();

    void LoadFile(std::string filename);
    YAML::Node GetRoot();
    YAML::Node GetSignature(std::string signature);

private:
    Configuration();

protected:
    YAML::Node rootNode_;
};

} // namespace chimera

#endif // __CHIMERA_CONFIGURATION_H__