#include "chimera/configuration.h"
#include "chimera/util.h"

using namespace clang;

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

void chimera::Configuration::Process(CompilerInstance *ci)
{
    // Resolve namespace configuration entries within provided AST.
    const auto namespaces = rootNode_["namespaces"];
    for(auto it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        std::string ns_str = it->as<std::string>();
        auto ns = chimera::util::resolveNamespace(ci, ns_str);
        if (ns)
        {
            std::cout << "Namespace: " << ns->getNameAsString() << std::endl;
            namespaces_.insert(ns);
        }
        else
        {
            std::cerr << "Unable to resolve namespace '"
                      << ns_str << "'." << std::endl;
            continue;
        }
    }

    // Resolve namespace configuration entries within provided AST.
    const auto declarations = rootNode_["declarations"];
    for(auto it = declarations.begin(); it != declarations.end(); ++it)
    {
        std::string decl_str = it->first.as<std::string>();
        auto decl = chimera::util::resolveDeclaration(ci, decl_str);
        if (decl)
        {
            std::cout << "Declaration: " << decl->getNameAsString() << std::endl;
        }
        else
        {
            std::cerr << "UNKNOWN TYPE: " << decl_str << std::endl;
        }
    }
}

const YAML::Node& chimera::Configuration::GetRoot() const
{
    return rootNode_;
}

const std::set<const clang::NamedDecl*>& chimera::Configuration::GetNamespaces() const
{
    return namespaces_;
}

const YAML::Node& chimera::Configuration::GetDeclaration(const clang::Decl *decl) const
{
    const auto d = declarations_.find(decl->getCanonicalDecl());
    return d != declarations_.end() ? d->second : emptyNode_;
}
