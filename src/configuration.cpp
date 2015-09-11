#include "chimera/configuration.h"

chimera::Configuration::Configuration()
{
    // Do nothing.
}

chimera::Configuration& chimera::Configuration::GetInstance()
{
    static chimera::Configuration config;
    return config;
}

void chimera::Configuration::LoadFile(std::string filename)
{
    try
    {
        rootNode_ = YAML::LoadFile(filename);
    }
    catch(YAML::Exception& e) 
    {
        // If unable to read the configuration YAML, terminate with an error.
        std::cerr << "Unable to read configuration '" << filename << "'."
                  << std::endl << e.what() << std::endl;
        exit(-1);
    }
}

YAML::Node chimera::Configuration::GetRoot()
{
    return rootNode_;
}

YAML::Node chimera::Configuration::GetSignature(std::string signature)
{
    // TODO: lookup by function decl.
    YAML::Node emptyNode;
    return emptyNode;
}
