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

/*
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

    // Resolve customizable snippets that will be inserted into the file.
    const YAML::Node &template_config = parent_.GetRoot()["template"]["main"];
    std::string header_snippet, postinclude_snippet, footer_snippet, precontent_snippet;
    getSnippet(template_config["header"],
               parent_.GetConfigFilename(), header_snippet);
    getSnippet(template_config["postinclude"],
               parent_.GetConfigFilename(), postinclude_snippet);
    getSnippet(template_config["precontent"],
               parent_.GetConfigFilename(), precontent_snippet);
    getSnippet(template_config["footer"],
               parent_.GetConfigFilename(), footer_snippet);

    // Create a stream wrapper to write header and footer of file.
    std::cout << binding_filename << std::endl;
    binding_.reset(new chimera::Stream(
        stream, binding_prototype, includes_,
        header_snippet, postinclude_snippet, footer_snippet));
    *binding_ << precontent_snippet << "\n";
    */
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
    const auto canonical_type = type.getCanonicalType();
    for (const auto &entry : types_)
    {
        if (entry.first == canonical_type)
            return entry.second;
    }
    return emptyNode_;
}

std::string
chimera::CompiledConfiguration::GetConstant(const std::string &value) const
{
    const YAML::Node &constants = parent_.GetRoot()["constants"];
    return constants[value].as<std::string>(value);
}

clang::CompilerInstance *chimera::CompiledConfiguration::GetCompilerInstance() const
{
    return ci_;
}

clang::ASTContext &chimera::CompiledConfiguration::GetContext() const
{
    return ci_->getASTContext();
}

bool chimera::CompiledConfiguration::IsEnclosed(const clang::Decl *decl) const
{
    // Filter over the namespaces and only traverse ones that are enclosed
    // by one of the configuration namespaces.
    for (const auto &it : GetNamespaces())
    {
        if (decl->getDeclContext() && it->Encloses(decl->getDeclContext()))
        {
            return true;
        }
    }
    return false;
}

bool
chimera::CompiledConfiguration::Render(std::string view, std::string key,
                                       const std::shared_ptr<mstch::object> &context) const
{
    // Get the mangled name property if it exists.
    if (!context->has("mangled_name"))
    {
        std::cerr << "Cannot serialize template with no mangled name." << std::endl;
        return false;
    }
    std::string mangled_name = ::mstch::render("{{mangled_name}}", context);

    // Create an output file depending on the provided parameters.
    std::string binding_filename =
        parent_.GetOutputPath() + "/" + mangled_name + ".cpp";
    auto stream = ci_->createOutputFile(
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
                  << "'" << ::mstch::render("{{name}}", context) << "'."
                  << std::endl;
        return false;
    }

    // Resolve customizable snippets that will be inserted into the file.
    const YAML::Node &template_config = parent_.GetRoot()["template"]["file"];
    std::string header_snippet, postinclude_snippet, footer_snippet;
    getSnippet(template_config["header"],
               parent_.GetConfigFilename(), header_snippet);
    getSnippet(template_config["postinclude"],
               parent_.GetConfigFilename(), postinclude_snippet);
    getSnippet(template_config["footer"],
               parent_.GetConfigFilename(), footer_snippet);

    // Augment top-level context as necessary.
    ::mstch::map full_context {
        {"header", header_snippet},
        {"postinclude", postinclude_snippet},
        {"footer", footer_snippet},
        {key, context}
    };

    // Render the mstch template to the given output file.
    *stream << ::mstch::render(view, full_context);
    std::cout << binding_filename << std::endl;
    return true;
}

/**
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
*/