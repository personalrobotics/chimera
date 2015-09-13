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
    /*
    const auto namespaces = rootNode_["namespaces"];
    for(auto it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        std::string ns_str = it->as<std::string>();

        clang::IdentifierInfo &ident_info = Context.Idents.get(ns_str);
        auto decl_name = Context.DeclarationNames.getIdentifier(&ident_info);
        auto result = Context.getTranslationUnitDecl()->lookup(decl_name);

        if (result.empty())
        {
            std::cerr << "Unable to resolve namespace '"
                      << ns_str << "'." << std::endl;
            continue;
        }

        for(auto ns_it = result.begin(); ns_it != result.end(); ++ns_it)
        {
            namespaces_.insert(*ns_it);
        }
    }
    */

    // Resolve namespace configuration entries within provided AST.
    const auto declarations = rootNode_["declarations"];
    for(auto it = declarations.begin(); it != declarations.end(); ++it)
    {
        std::string decl_str = it->first.as<std::string>();
        auto typeDecl = chimera::util::findType(ci, decl_str);
        std::cout << "Type: " << typeDecl.getAsString() << std::endl;
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

const YAML::Node& chimera::Configuration::GetSignature(const clang::Decl& declaration) const
{
    // TODO: lookup by function decl.
    return emptyNode_;
}
