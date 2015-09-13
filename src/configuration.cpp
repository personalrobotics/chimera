#include "chimera/configuration.h"
#include "chimera/util.h"

using namespace clang;

/**
 * Counts the number of whitespace-separated words in a string.
 *
 * See: http://stackoverflow.com/a/3672259 
 */
size_t countWordsInString(const std::string & str)
{
    std::stringstream stream(str);
    return std::distance(std::istream_iterator<std::string>(stream),
                         std::istream_iterator<std::string>());
}

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

std::unique_ptr<chimera::CompiledConfiguration>
chimera::Configuration::Process(CompilerInstance *ci) const
{
    std::unique_ptr<chimera::CompiledConfiguration> config(new CompiledConfiguration());

    // Resolve namespace configuration entries within provided AST.
    const auto namespaces = rootNode_["namespaces"];
    for(auto it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        std::string ns_str = it->as<std::string>();
        auto ns = chimera::util::resolveNamespace(ci, ns_str);
        if (ns)
        {
            std::cout << "Namespace: " << ns->getNameAsString() << std::endl;
            config->namespaces_.insert(ns);
        }
        else
        {
            std::cerr << "Unable to resolve namespace: "
                      << "'" << ns_str << "'." << std::endl;
        }
    }

    // Resolve namespace configuration entries within provided AST.
    const auto declarations = rootNode_["declarations"];
    for(auto it = declarations.begin(); it != declarations.end(); ++it)
    {
        std::string decl_str = it->first.as<std::string>();

        // If there are multiple words, assume a full declaration.
        // If there is only one word, assume a record declaration.
        auto decl = (countWordsInString(decl_str) == 1)
                    ? chimera::util::resolveRecord(ci, decl_str)
                    : chimera::util::resolveDeclaration(ci, decl_str);
        if (decl)
        {
            std::cout << "Declaration: " << decl->getNameAsString() << std::endl;
            config->declarations_[decl] = it->second;
        }
        else
        {
            std::cerr << "Unable to resolve declaration: "
                      << "'" << decl_str << "'" << std::endl;
        }
    }

    return config;
}

const YAML::Node& chimera::Configuration::GetRoot() const
{
    return rootNode_;
}

const YAML::Node chimera::CompiledConfiguration::emptyNode_;

chimera::CompiledConfiguration::CompiledConfiguration()
{
    // Do nothing.
}

const std::set<const clang::NamespaceDecl*>& chimera::CompiledConfiguration::GetNamespaces() const
{
    return namespaces_;
}

const YAML::Node& chimera::CompiledConfiguration::GetDeclaration(const clang::Decl *decl) const
{
    const auto d = declarations_.find(decl->getCanonicalDecl());
    return d != declarations_.end() ? d->second : emptyNode_;
}
