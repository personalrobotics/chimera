#include <iostream>
#include <llvm/Support/raw_ostream.h>
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
        rootFilename_ = filename;
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
    std::unique_ptr<chimera::CompiledConfiguration> config(new CompiledConfiguration(ci));

    // Resolve namespace configuration entries within provided AST.
    for(const auto &it : rootNode_["namespaces"])
    {
        std::string ns_str = it.as<std::string>();
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
    for(const auto &it : rootNode_["declarations"])
    {
        std::string decl_str = it.first.as<std::string>();

        // If there are multiple words, assume a full declaration.
        // If there is only one word, assume a record declaration.
        auto decl = (countWordsInString(decl_str) == 1)
                    ? chimera::util::resolveRecord(ci, decl_str)
                    : chimera::util::resolveDeclaration(ci, decl_str);
        if (decl)
        {
            std::cout << "Declaration: " << decl->getNameAsString() << std::endl;
            config->declarations_[decl] = it.second;
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

const std::string &chimera::Configuration::GetFilename() const
{
    return rootFilename_;
}

const YAML::Node chimera::CompiledConfiguration::emptyNode_;

chimera::CompiledConfiguration::CompiledConfiguration(CompilerInstance *ci)
: ci_(ci)
, mangler_(ci->getASTContext().createMangleContext())
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

std::unique_ptr<llvm::raw_pwrite_stream>
chimera::CompiledConfiguration::GetOutputFile(const clang::Decl *decl) const
{
    // Try to convert to a canonical named declaration.
    const auto canonical_decl = decl->getCanonicalDecl();
    if (!isa<clang::NamedDecl>(canonical_decl))
    {
        std::cerr << "Cannot serialize unnamed declaration." << std::endl;
        canonical_decl->dumpColor();
        return nullptr;
    }
    const auto named_decl = cast<clang::NamedDecl>(canonical_decl);

    // Use the C++ mangler to create the mangled base input name.
    llvm::SmallString<1024> base_input_buffer;
    llvm::raw_svector_ostream base_input_stream(base_input_buffer);
    mangler_->mangleName(named_decl, base_input_stream);

    // Create an output file depending on the provided parameters.
    // TODO: In newer Clang versions, this function returns std::unique<>.
    return std::unique_ptr<llvm::raw_pwrite_stream>(ci_->createOutputFile(
        ci_->getFrontendOpts().OutputFile, // Output Path
        false, // Open the file in binary mode
        false, // Register with llvm::sys::RemoveFileOnSignal
        base_input_stream.str(), // If no OutputPath, a name to derive output path
        ".cpp", // The extension to use for derived name.
        false, // Use a temporary file that should be renamed
        false // Create missing directories in the output path
    ));
    return nullptr;
}
