#include "chimera/configuration.h"
#include "chimera/mstch.h"
#include "chimera/util.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>

using namespace clang;

namespace
{

/**
 * Map of counts of each long prefix encountered.
 *
 * This allows the generation of safe incrementing filenames in the case
 * that many long paths with the same prefix are encountered.
 */
std::map<std::string, int> large_filename_prefixes;

constexpr int MAX_PATH_LENGTH = 255;
constexpr int MAX_COUNTER_LENGTH = 4;

/**
 * De-conflicts paths that are longer than 255 characters.
 * (This is the maximum path length on many operating systems.)
 */
std::string sanitizePath(const std::string &path)
{
    // If the path length is short, just return it.
    if (path.size() < MAX_PATH_LENGTH)
        return path;

    // If the path length is long, compute a safe prefix and append an
    // incrementing counter variable to the end of the filename.
    size_t suffix_index = path.find_last_of(".");
    const std::string path_suffix = (suffix_index == std::string::npos) ?
                                    "" : path.substr(suffix_index);
    const int prefix_size = std::max(
        0, MAX_PATH_LENGTH - MAX_COUNTER_LENGTH - (int)path_suffix.size() - 2);
    const std::string path_prefix = path.substr(0, prefix_size);
    const int index = large_filename_prefixes[path_prefix]++;

    // Create the new filename as "prefix_{index}.suffix"
    std::stringstream ss;
    ss << path_prefix << "_";
    ss << std::setfill('0') << std::setw(MAX_COUNTER_LENGTH) << index;
    if (path_suffix.length())
        ss << path_suffix;
    return ss.str();
}

} // namespace


const YAML::Node chimera::CompiledConfiguration::emptyNode_(
    YAML::NodeType::Undefined);

chimera::Configuration::Configuration()
: outputPath_(".")
, outputModuleName_("chimera_binding")
{
    // Instantiate all bindings.
    // TODO: is there a better place to be doing this?
    chimera::binding::initializeBuiltinBindings();
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

void chimera::Configuration::AddInputNamespaceName(const std::string &namespaceName)
{
    inputNamespaceNames_.push_back(namespaceName);
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
, configNode_(parent.GetRoot()) // TODO: do we need this reference?
, bindingNode_(configNode_["template"]) // TODO: is this always ok?
, ci_(ci)
{
    // Start out by setting the binding name to the default.
    std::string binding_name = chimera::binding::DEFAULT_NAME;

    // Resolve command-line namespaces.  Since these cannot include
    // configuration information, they are simpler to handle.
    for (const std::string &ns_str : parent.inputNamespaceNames_)
    {
        auto ns = chimera::util::resolveNamespace(ci, ns_str);
        if (!ns)
        {
            std::cerr << "Unable to resolve namespace: "
                      << "'" << ns_str << "'." << std::endl;
            continue;
        }
        namespaces_.insert(ns);
    }

    // Get a reference to the configuration YAML root node.
    // If it is not available, then skip configuration node parsing.
    if (configNode_)
    {
        // Parse 'namespaces' section of configuration YAML if it exists.
        const YAML::Node &namespacesNode = configNode_["namespaces"];
        if (namespacesNode)
        {
            // Check that 'namespaces' node in configuration YAML is a map.
            if (!namespacesNode.IsMap())
            {
                std::cerr << "'namespaces' in configuration YAML must be a map."
                          << std::endl;
                exit(-2);
            }

            // Resolve namespace configuration entries within provided AST.
            for(const auto &it : namespacesNode)
            {
                std::string ns_str = it.first.as<std::string>();
                auto ns = chimera::util::resolveNamespace(ci, ns_str);
                if (ns)
                {
                    declarations_[ns] = it.second;
                    namespaces_.insert(ns);
                }
                else
                {
                    std::cerr << "Unable to resolve namespace: "
                              << "'" << ns_str << "'." << std::endl;
                    exit(-2);
                }
            }
        }

        // Parse 'classes' section of configuration YAML if it exists.
        const YAML::Node &classesNode = configNode_["classes"];
        if (classesNode)
        {
            // Check that 'classes' node in configuration YAML is a map.
            if (!classesNode.IsMap())
            {
                std::cerr << "'classes' in configuration YAML must be a map."
                          << std::endl;
                exit(-2);
            }

            // Resolve class/struct configuration entries within provided AST.
            for(const auto &it : configNode_["classes"])
            {
                std::string decl_str = it.first.as<std::string>();
                auto decl = chimera::util::resolveRecord(ci, decl_str);
                if (decl)
                {
                    declarations_[decl] = it.second;
                }
                else
                {
                    std::cerr << "Unable to resolve class declaration: "
                              << "'" << decl_str << "'" << std::endl;
                    exit(-2);
                }
            }
        }

        // Parse 'functions' section of configuration YAML if it exists.
        const YAML::Node &functionsNode = configNode_["functions"];
        if (functionsNode)
        {
            // Check that 'functions' node in configuration YAML is a map.
            if (!functionsNode.IsMap())
            {
                std::cerr << "'functions' in configuration YAML must be a map."
                          << std::endl;
                exit(-2);
            }

            // Resolve function configuration entries within provided AST.
            for(const auto &it : functionsNode)
            {
                std::string decl_str = it.first.as<std::string>();
                auto decl = chimera::util::resolveDeclaration(ci, decl_str);
                if (decl)
                {
                    declarations_[decl] = it.second;
                }
                else
                {
                    std::cerr << "Unable to resolve function declaration: "
                              << "'" << decl_str << "'" << std::endl;
                    exit(-2);
                }
            }
        }

        // Parse 'types' section of configuration YAML if it exists.
        const YAML::Node &typesNode = configNode_["types"];
        if (typesNode)
        {
            // Check that 'types' node in configuration YAML is a map.
            if (!typesNode.IsMap())
            {
                std::cerr << "'types' in configuration YAML must be a map."
                          << std::endl;
                exit(-2);
            }

            // Resolve type configuration entries within provided AST.
            for(const auto &it : typesNode)
            {
                std::string type_str = it.first.as<std::string>();
                auto type = chimera::util::resolveType(ci, type_str);
                if (type.getTypePtrOrNull())
                {
                    types_.emplace_back(std::make_pair(type, it.second));
                }
                else
                {
                    std::cerr << "Unable to resolve type: "
                              << "'" << type_str << "'" << std::endl;
                    exit(-2);
                }
            }
        }

        // Parse 'binding' section of configuration YAML if it exists.
        const YAML::Node &bindingNode = configNode_["binding"];
        if (bindingNode)
        {
            // Check that 'binding' node in configuration YAML is a scalar.
            if (!typesNode.IsScalar())
            {
                std::cerr << "'binding' in configuration YAML must be a map."
                          << std::endl;
                exit(-2);
            }
            binding_name = bindingNode.as<std::string>();
        }
    }

    // Resolve the base binding definition from the specified binding name.
    const auto bindingIt = chimera::binding::DEFINITIONS.find(binding_name);
    if (bindingIt == chimera::binding::DEFINITIONS.end())
    {
        std::cerr << "Unable to resolve binding definition: "
                  << "'" << binding_name << "'" << std::endl;
        exit(-2);
    }
    bindingDefinition_ = bindingIt->second;

    // Override individual templates if specified in the configuration.
    if (bindingNode_)
    {
        if (const YAML::Node &classTemplateNode = bindingNode_["class"])
            bindingDefinition_.class_cpp = Lookup(classTemplateNode);

        if (const YAML::Node &enumTemplateNode = bindingNode_["enum"])
            bindingDefinition_.enum_cpp = Lookup(enumTemplateNode);

        if (const YAML::Node &functionTemplateNode = bindingNode_["function"])
            bindingDefinition_.function_cpp = Lookup(functionTemplateNode);

        if (const YAML::Node &moduleTemplateNode = bindingNode_["module"])
            bindingDefinition_.module_cpp = Lookup(moduleTemplateNode);

        if (const YAML::Node &variableTemplateNode = bindingNode_["variable"])
            bindingDefinition_.variable_cpp = Lookup(variableTemplateNode);
    }

    // Set custom escape function that disables HTML escaping on mstch output.
    //
    // This is not desirable in chimera because many C++ types include characters
    // that can be accidentally escaped, such as `<>` and `&`.
    //
    // See: https://github.com/no1msd/mstch#custom-escape-function
    //
    ::mstch::config::escape = [](const std::string& str) -> std::string {
        return str;
    };
}

chimera::CompiledConfiguration::~CompiledConfiguration()
{
    // Create and sanitize path and filename of top-level source file.
    // Because we may compress the filename to fit OS character limits,
    // we generate the full path, then split the filename from it.
    const std::string binding_path =
        sanitizePath(parent_.GetOutputPath() + "/" +
                     parent_.GetOutputModuleName() + ".cpp");
    size_t path_index = binding_path.find_last_of("/");
    const std::string binding_filename =
        (path_index == std::string::npos) ?
            "" : binding_path.substr(path_index + 1);

    // Create an output file depending on the provided parameters.
    auto stream = ci_->createOutputFile(
        binding_path,
        false, // Open the file in binary mode
        false, // Register with llvm::sys::RemoveFileOnSignal
        "", // The derived basename (shouldn't be used)
        "", // The extension to use for derived name (shouldn't be used)
        false, // Use a temporary file that should be renamed
        false // Create missing directories in the output path
    );

    // If file creation failed, report the error and fail immediately.
    if (!stream)
    {
        std::cerr << "Failed to create top-level output file "
                  << "'" << binding_path << "'."
                  << std::endl;
        exit(-4);
    }

    // Create collections for the ordered sets of bindings and namespaces.
    ::mstch::array binding_names(binding_names_.begin(),
                                 binding_names_.end());
    ::mstch::array binding_namespaces;
    for (const clang::NamespaceDecl *namespace_decl : binding_namespaces_)
    {
        binding_namespaces.push_back(
            std::make_shared<chimera::mstch::Namespace>(
                *this, namespace_decl));
    }

    // Create a top-level context that contains the extracted information
    // about the module.
    ::mstch::map full_context {
        {"module", ::mstch::map {
            {"name", parent_.GetOutputModuleName()},
            {"bindings", binding_names},
            // Note: binding namespaces will be lexically ordered.
            {"namespaces", binding_namespaces}
        }}
    };

    // Resolve customizable snippets that will be inserted into the file
    // from the configuration file's "template::main" entry.
    if (bindingNode_)
    {
        chimera::util::extendWithYAMLNode(
            full_context, bindingNode_["main"], false,
            std::bind(&chimera::CompiledConfiguration::Lookup,
                      this, std::placeholders::_1)
        );
    }

    // Render the mstch template to the given output file.
    *stream << ::mstch::render(bindingDefinition_.module_cpp, full_context);
    std::cout << binding_filename << std::endl;
}

void
chimera::CompiledConfiguration::AddTraversedNamespace(const clang::NamespaceDecl* decl)
{
    binding_namespaces_.insert(decl->getCanonicalDecl());
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

bool chimera::CompiledConfiguration::IsSuppressed(const QualType type) const
{
    return (chimera::CompiledConfiguration::GetType(type).IsNull());
}

bool chimera::CompiledConfiguration::IsSuppressed(const clang::Decl *decl) const
{
    const auto config = chimera::CompiledConfiguration::GetDeclaration(decl);

    // If the declaration is directly suppressed, report this.
    if (config.IsNull())
        return true;

    // Functions can be suppressed if they return a suppressed type.
    if (isa<FunctionDecl>(decl) && !config["return_value_policy"])
    {
        const FunctionDecl *function_decl = cast<FunctionDecl>(decl);
        const QualType return_qual_type =
            chimera::util::getFullyQualifiedType(
                GetContext(), function_decl->getReturnType());
        return IsSuppressed(return_qual_type);
    }
    // Fields can be suppressed if they represent a suppressed type.
    else if (isa<FieldDecl>(decl) && !config["return_value_policy"])
    {
        const FieldDecl *field_decl = cast<FieldDecl>(decl);
        const QualType value_qual_type =
            chimera::util::getFullyQualifiedType(
                GetContext(), field_decl->getType());
        return IsSuppressed(value_qual_type);
    }

    // If no other rules apply, report that the declaration was not suppressed.
    return false;
}

std::string chimera::CompiledConfiguration::Lookup(const YAML::Node &node) const
{
    // If the node is not scalar, we cannot load it, so return the default.
    if (!node.IsScalar())
        return "";

    // If the node type tag is "file" then load the contents of a file.
    if (node.Tag() == "file")
    {
        // Get reference to path to configuration file itself.
        const std::string &config_path = parent_.GetConfigFilename();

        // Concatenate YAML filepath with source relative path.
        // TODO: this is somewhat brittle.
        std::string source_path = node.as<std::string>();

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
            exit(-5);
        }

        // Copy file content to the output stream.
        std::string snippet;
        snippet.assign(std::istreambuf_iterator<char>(source),
                       std::istreambuf_iterator<char>());
        return snippet;
    }

    // Otherwise the node simply contains a string, so return it.
    return node.as<std::string>();
}

bool
chimera::CompiledConfiguration::Render(std::string view, std::string key,
                                       const std::shared_ptr<::mstch::object> &context)
{
    // Get the mangled name property if it exists.
    if (!context->has("mangled_name"))
    {
        std::cerr << "Cannot serialize template with no mangled name." << std::endl;
        return false;
    }
    std::string mangled_name = ::mstch::render("{{mangled_name}}", context);

    // Create and sanitize path and filename of top-level source file.
    // Because we may compress the filename to fit OS character limits,
    // we generate the full path, then split the filename from it.
    const std::string binding_path =
        sanitizePath(parent_.GetOutputPath() + "/" + mangled_name + ".cpp");
    size_t path_index = binding_path.find_last_of("/");
    const std::string binding_filename =
        (path_index == std::string::npos) ?
            "" : binding_path.substr(path_index + 1);

    // Create an output file depending on the provided parameters.
    auto stream = ci_->createOutputFile(
        binding_path,
        false, // Open the file in binary mode
        false, // Register with llvm::sys::RemoveFileOnSignal
        "", // The derived basename (shouldn't be used)
        "", // The extension to use for derived name (shouldn't be used)
        false, // Use a temporary file that should be renamed
        false // Create missing directories in the output path
    );

    // If file creation failed, report the error and fail immediately.
    if (!stream)
    {
        std::cerr << "Failed to create output file "
                  << "'" << binding_path << "'"
                  << " for "
                  << "'" << ::mstch::render("{{name}}", context) << "'."
                  << std::endl;
        exit(-6);
    }

    // Create a top-level context that contains the extracted information
    // about this particular binding component.
    ::mstch::map full_context {
        {key, context}
    };

    // Resolve customizable snippets that will be inserted into the file
    // from the configuration file's "template::file" entry.
    if (bindingNode_)
    {
        chimera::util::extendWithYAMLNode(
            full_context, bindingNode_["file"], false,
            std::bind(&chimera::CompiledConfiguration::Lookup,
                      this, std::placeholders::_1)
        );
    }

    // Render the mstch template to the given output file.
    *stream << ::mstch::render(view, full_context);
    std::cout << binding_filename << std::endl;

    // Record this binding name for use at the top-level.
    binding_names_.push_back(mangled_name);
    return true;
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::CXXRecord> context)
{
    return Render(bindingDefinition_.class_cpp, "class", context);
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::Enum> context)
{
    return Render(bindingDefinition_.enum_cpp, "enum", context);
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::Function> context)
{
    return Render(bindingDefinition_.function_cpp, "function", context);
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::Variable> context)
{
    return Render(bindingDefinition_.variable_cpp, "variable", context);
}
