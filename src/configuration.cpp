#include "chimera/configuration.h"
#include "chimera/util.h"

#include <fstream>
#include <iostream>

using namespace clang;

namespace
{

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

bool getSnippet(const YAML::Node &node, const std::string &config_path,
                std::string &snippet)
{
    if (const YAML::Node &content_node = node["content"])
    {
        snippet = content_node.as<std::string>();
        return true;
    }
    else if (const YAML::Node &source_node = node["source"])
    {
        // Concatenate YAML filepath with source relative path.
        // TODO: this is somewhat brittle.
        std::string source_path = source_node.as<std::string>();

        if (source_path.front() != '/')
        {
            std::size_t found = config_path.rfind("/");
            if (found != std::string::npos)
            {
                source_path = config_path.substr(0, found) + "/" + source_path;
            }
            else
            {
                source_path = "./" + source_path;
            }
        }

        // Try to open configuration file.
        std::ifstream source(source_path);
        if (source.fail())
        {
            std::cerr << "Warning: Failed to open source '"
                      << source_path << "': " << strerror(errno) << std::endl;
            return true;
        }

        // Copy file content to the output stream.
        snippet.assign(std::istreambuf_iterator<char>(source),
                       std::istreambuf_iterator<char>());

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace


const YAML::Node chimera::CompiledConfiguration::emptyNode_(
    YAML::NodeType::Undefined);

chimera::Configuration::Configuration()
: outputPath_(".")
, outputModuleName_("chimera_binding")
{
    // Do nothing.
}

chimera::Configuration& chimera::Configuration::GetInstance()
{
    static chimera::Configuration config;
    return config;
}

void chimera::Configuration::LoadFile(const std::string &filename)
{
    try
    {
        configNode_ = YAML::LoadFile(filename);
        configFilename_ = filename;
    }
    catch(YAML::Exception& e) 
    {
        // If unable to read the configuration YAML, terminate with an error.
        std::cerr << "Unable to read configuration '" << filename << "'."
                  << std::endl << e.what() << std::endl;
        exit(-1);
    }
}

void chimera::Configuration::SetOutputPath(const std::string &path)
{
    // Setting the path to the empty string makes no sense and will break the
    // binding path concatenation, so if an empty string is passed, assume the
    // caller wanted to reset back to the default.
    outputPath_ = path.empty() ? "." : path;
}

void chimera::Configuration::SetOutputModuleName(const std::string &moduleName)
{
    // Setting the module name to the empty string makes no sense and will
    // break the binding, so if an empty string is passed, assume the caller
    // wanted to reset back to the default.
    outputModuleName_ = moduleName.empty() ? "chimera_binding" : moduleName;
}

std::unique_ptr<chimera::CompiledConfiguration>
chimera::Configuration::Process(CompilerInstance *ci) const
{
    return std::unique_ptr<chimera::CompiledConfiguration>(
        new CompiledConfiguration(*this, ci));
}

const YAML::Node& chimera::Configuration::GetRoot() const
{
    return configNode_;
}

const std::string &chimera::Configuration::GetConfigFilename() const
{
    return configFilename_;
}

const std::string &chimera::Configuration::GetOutputPath() const
{
    return outputPath_;
}

const std::string &chimera::Configuration::GetOutputModuleName() const
{
    return outputModuleName_;
}

chimera::CompiledConfiguration::CompiledConfiguration(
    const chimera::Configuration &parent, CompilerInstance *ci)
: parent_(parent)
, ci_(ci)
, mangler_(ci->getASTContext().createMangleContext())
{   
    // Get a reference to the configuration YAML structure.
    const YAML::Node &configNode = parent.GetRoot();

    // Compile the list of input files into the list of includes.
    for (const auto &input_file : ci->getInvocation().getFrontendOpts().Inputs)
        includes_.push_back(input_file.getFile());

    // Resolve namespace configuration entries within provided AST.
    for(const auto &it : configNode["namespaces"])
    {
        std::string ns_str = it.as<std::string>();
        auto ns = chimera::util::resolveNamespace(ci, ns_str);
        if (ns)
        {
            namespaces_.insert(ns);
        }
        else
        {
            std::cerr << "Unable to resolve namespace: "
                      << "'" << ns_str << "'." << std::endl;
        }
    }

    // Resolve namespace configuration entries within provided AST.
    for(const auto &it : configNode["declarations"])
    {
        std::string decl_str = it.first.as<std::string>();

        // If there are multiple words, assume a full declaration.
        // If there is only one word, assume a record declaration.
        auto decl = (countWordsInString(decl_str) == 1)
                     ? chimera::util::resolveRecord(ci, decl_str)
                     : chimera::util::resolveDeclaration(ci, decl_str);
        if (decl)
        {
            declarations_[decl] = it.second;
        }
        else
        {
            std::cerr << "Unable to resolve declaration: "
                      << "'" << decl_str << "'" << std::endl;
        }
    }

    // Resolve type configuration entries within provided AST.
    for(const auto &it : configNode["types"])
    {
        std::string type_str = it.first.as<std::string>();
        auto type = chimera::util::resolveType(ci, type_str);
        if (type.getTypePtrOrNull())
        {
            types_.push_back(std::make_pair(type, it.second));
        }
        else
        {
            std::cerr << "Unable to resolve type: "
                      << "'" << type_str << "'" << std::endl;
        }
    }

    // Create the top-level binding source file.
    std::string binding_filename =
        parent_.GetOutputPath() + "/" + parent_.GetOutputModuleName() + ".cpp";
    std::string binding_prototype =
        "BOOST_PYTHON_MODULE(" + parent_.GetOutputModuleName() + ")";
    // TODO: In newer Clang versions, this function returns std::unique<>.
    auto *stream = ci_->createOutputFile(
        binding_filename,
        false, // Open the file in binary mode
        false, // Register with llvm::sys::RemoveFileOnSignal
        "", // The derived basename (shouldn't be used)
        "", // The extension to use for derived name (shouldn't be used)
        false, // Use a temporary file that should be renamed
        false // Create missing directories in the output path
    );

    // Create a reference to the binding file.
    std::cout << binding_filename << std::endl;
    binding_.reset(new chimera::Stream(stream, binding_prototype, includes_));
}

const std::set<const clang::NamespaceDecl*>&
chimera::CompiledConfiguration::GetNamespaces() const
{
    return namespaces_;
}

const YAML::Node&
chimera::CompiledConfiguration::GetDeclaration(const clang::Decl *decl) const
{
    const auto d = declarations_.find(decl->getCanonicalDecl());
    return d != declarations_.end() ? d->second : emptyNode_;
}

const YAML::Node&
chimera::CompiledConfiguration::GetType(const clang::QualType type) const
{
    const auto canonical_type =
        chimera::util::getFullyQualifiedType(ci_->getASTContext(), type);
    for (const auto &entry : types_)
    {
        if (entry.first == canonical_type)
            return entry.second;
    }
    return emptyNode_;
}

std::unique_ptr<chimera::Stream>
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

    // Use the C++ mangler to create the mangled binding filename.
    llvm::SmallString<1024> base_input_buffer;
    llvm::raw_svector_ostream base_input_stream(base_input_buffer);
    mangler_->mangleName(named_decl, base_input_stream);
    std::string mangled_name = base_input_stream.str();

    // Create an output file depending on the provided parameters.
    std::string binding_filename =
        parent_.GetOutputPath() + "/" + mangled_name + ".cpp";
    std::string binding_prototype =
        "void " + mangled_name + "()";
    // TODO: In newer Clang versions, this function returns std::unique<>.
    auto *stream = ci_->createOutputFile(
        binding_filename,
        false, // Open the file in binary mode
        false, // Register with llvm::sys::RemoveFileOnSignal
        "", // The derived basename (shouldn't be used)
        "", // The extension to use for derived name (shouldn't be used)
        false, // Use a temporary file that should be renamed
        false // Create missing directories in the output path
    );

    // If file creation failed, report the error and return a nullptr.
    if (!stream)
    {
        std::cerr << "Failed to create output file "
                  << "'" << binding_filename << "'"
                  << " for "
                  << "'" << named_decl->getQualifiedNameAsString() << "'."
                  << std::endl;
        return nullptr;
    }

    const YAML::Node &template_config = parent_.GetRoot()["template"];

    std::string header_snippet, postinclude_snippet, footer_snippet;
    getSnippet(template_config["header"],
               parent_.GetConfigFilename(), header_snippet);
    getSnippet(template_config["postinclude"],
               parent_.GetConfigFilename(), postinclude_snippet);
    getSnippet(template_config["footer"],
               parent_.GetConfigFilename(), footer_snippet);

    // Add this function to the top-level binding source file.
    *binding_ << "  " << binding_prototype << ";\n";
    *binding_ << "  " << mangled_name << "();\n\n";

    // Create a stream wrapper to write header and footer of file.
    std::cout << binding_filename << std::endl;
    return std::unique_ptr<chimera::Stream>(new chimera::Stream(
        stream, binding_prototype, includes_,
        header_snippet, postinclude_snippet, footer_snippet));
}

bool chimera::CompiledConfiguration::DumpOverride(
    const clang::Decl *decl, chimera::Stream &stream) const
{
    const YAML::Node &node = GetDeclaration(decl);

    std::string snippet;
    if (getSnippet(node, parent_.GetConfigFilename(), snippet))
    {
        stream << snippet << '\n';
        return true;
    }
    return false;
}
