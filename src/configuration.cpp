#include "chimera/configuration.h"
#include "chimera/mstch.h"
#include "chimera/util.h"

// TODO: Clean this up and move to something other than a header.
#include "chimera/boost_python_mstch.h"

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
, ci_(ci)
{
    // Get a reference to the configuration YAML structure.
    const YAML::Node &configNode = parent.configNode_;

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

    // Resolve namespace configuration entries within provided AST.
    for(const auto &it : configNode["namespaces"])
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
            exit(-1);
        }
    }

    // Resolve class/struct configuration entries within provided AST.
    for(const auto &it : configNode["classes"])
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

    // Resolve function configuration entries within provided AST.
    for(const auto &it : configNode["functions"])
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
            exit(-3);
        }
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

    // Resolve customizable snippets that will be inserted into the file.
    // Augment top-level context as necessary.
    const YAML::Node &template_config = parent_.GetRoot()["template"]["main"];
    ::mstch::map full_context {
        {"header", Lookup(template_config["header"])},
        {"precontent", Lookup(template_config["precontent"])},
        {"prebody", Lookup(template_config["prebody"])},
        {"footer", Lookup(template_config["footer"])},
        {"module", ::mstch::map {
            {"name", parent_.GetOutputModuleName()},
            {"bindings", binding_names},
            // Note: binding namespaces will be lexically ordered.
            {"namespaces", binding_namespaces}
        }}
    };

    // Render the mstch template to the given output file.
    std::string view = Lookup(parent_.GetRoot()["template"]["module"]);
    if (view.empty())
        view = MODULE_CPP;
    *stream << ::mstch::render(view, full_context);
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
    std::string config_path = parent_.GetConfigFilename();

    // If the node simply contains a string, return it.
    if (node.Type() == YAML::NodeType::Scalar)
        return node.as<std::string>();

    // If the node contains a "source:" entry, load from that.
    if (const YAML::Node &source_node = node["source"])
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
            exit(-5);
        }

        // Copy file content to the output stream.
        std::string snippet;
        snippet.assign(std::istreambuf_iterator<char>(source),
                       std::istreambuf_iterator<char>());
        return snippet;
    }

    // Return an empty string if not available.
    return "";
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

    // Resolve customizable snippets that will be inserted into the file.
    // Augment top-level context as necessary.
    const YAML::Node &template_config = parent_.GetRoot()["template"]["file"];
    ::mstch::map full_context {
        {"header", Lookup(template_config["header"])},
        {"precontent", Lookup(template_config["precontent"])},
        {"footer", Lookup(template_config["footer"])},
        {key, context}
    };

    // Render the mstch template to the given output file.
    *stream << ::mstch::render(view, full_context);
    std::cout << binding_filename << std::endl;

    // Record this binding name for use at the top-level.
    binding_names_.push_back(mangled_name);
    return true;
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::CXXRecord> context)
{
    std::string view = Lookup(parent_.GetRoot()["template"]["cxx_record"]);
    if (view.empty())
        view = CLASS_BINDING_CPP;
    return Render(view, "class", context);
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::Enum> context)
{
    std::string view = Lookup(parent_.GetRoot()["template"]["enum"]);
    if (view.empty())
        view = ENUM_BINDING_CPP;
    return Render(view, "enum", context);
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::Function> context)
{
    std::string view = Lookup(parent_.GetRoot()["template"]["function"]);
    if (view.empty())
        view = FUNCTION_BINDING_CPP;
    return Render(view, "function", context);
}

bool chimera::CompiledConfiguration::Render(const std::shared_ptr<chimera::mstch::Variable> context)
{
    std::string view = Lookup(parent_.GetRoot()["template"]["variable"]);
    if (view.empty())
        view = VAR_BINDING_CPP;
    return Render(view, "variable", context);
}
