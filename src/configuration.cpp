#include "chimera/configuration.h"
#include "chimera/mstch.h"
#include "chimera/util.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include <boost/optional.hpp>

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
    const std::string path_suffix
        = (suffix_index == std::string::npos) ? "" : path.substr(suffix_index);
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
  : outputPath_("."), outputModuleName_("chimera_binding"), strict_(false)
{
    // Do nothing.
}

void chimera::Configuration::LoadFile(const std::string &filename)
{
    try
    {
        configNode_ = YAML::LoadFile(filename);
        configFilename_ = filename;
    }
    catch (YAML::Exception &e)
    {
        // If unable to read the configuration YAML, terminate with an error.
        std::stringstream ss;
        ss << "Unable to read configuration '" << filename << "'.\n"
           << e.what();
        throw std::invalid_argument(ss.str());
    }
}

void chimera::Configuration::SetBindingName(const std::string &name)
{
    // Setting the name to the empty string makes no sense and will fail the
    // definition lookup, so we simply fail here to try to alert the user as
    // soon as possible to a possible parsing issue.
    if (name.empty())
    {
        throw std::invalid_argument(
            "Binding name cannot be set to empty string.");
    }
    bindingName_ = name;
}

void chimera::Configuration::SetOutputPath(const std::string &path)
{
    // Setting the path to the empty string makes no sense and will break the
    // binding path concatenation, so we simply fail here to try to alert the
    // user as soon as possible to a possible parsing issue.
    if (path.empty())
    {
        throw std::invalid_argument(
            "Output path cannot be set to empty string.");
    }
    outputPath_ = path;
}

void chimera::Configuration::SetOutputModuleName(const std::string &moduleName)
{
    // Setting the module name to the empty string makes no sense and will
    // break the binding, so we simply fail here to try to alert the
    // user as soon as possible to a possible parsing issue.
    if (moduleName.empty())
    {
        throw std::invalid_argument(
            "Module name cannot be set to empty string.");
    }
    outputModuleName_ = moduleName;
}

void chimera::Configuration::AddInputNamespaceName(
    const std::string &namespaceName)
{
    inputNamespaceNames_.push_back(namespaceName);
}

void chimera::Configuration::AddSourcePath(const std::string &sourcePath)
{
    inputSourcePaths_.push_back(sourcePath);
}

void chimera::Configuration::SetStrict(bool val)
{
    strict_ = val;
}

std::unique_ptr<chimera::CompiledConfiguration> chimera::Configuration::Process(
    CompilerInstance *ci) const
{
    return std::unique_ptr<chimera::CompiledConfiguration>(
        new CompiledConfiguration(*this, ci));
}

const YAML::Node &chimera::Configuration::GetRoot() const
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
  , configNode_(parent.GetRoot())         // TODO: do we need this reference?
  , bindingNode_(configNode_["template"]) // TODO: is this always ok?
  , ci_(ci)
{
    using chimera::util::lookupYAMLNode;

    // This placeholder will be filled in by the options.strict specified
    // in the configuration YAML if it exists, or remain unset otherwise.
    boost::optional<bool> config_options_strict;

    // Parse 'options.strict' tag in configuration YAML
    const YAML::Node strictNode
        = chimera::util::lookupYAMLNode(configNode_, "options", "strict");
    if (strictNode)
    {
        // Check that 'strict' node in configuration YAML is a scalar.
        if (!strictNode.IsScalar())
        {
            throw std::runtime_error(
                "'options.strict' in configuration YAML must be a scalar.");
        }
        config_options_strict = strictNode.as<bool>();
    }

    // Set the options.strict from one of the following sources in order of
    // priority: 1) CLI '-strict' setting (True if -strict passed, False
    // otherwise) 2) YAML configuration setting (True/False) 3) false by default
    if (parent.strict_)
    {
        strict_ = parent.strict_;
    }
    else if (config_options_strict)
    {
        strict_ = *config_options_strict;
    }
    else
    {
        strict_ = false;
    }

    // This placeholder will be filled in by the binding name specified
    // in the configuration YAML if it exists, or remain empty otherwise.
    std::string config_binding_name;

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
        namespacesIncluded_.insert(ns);
    }

    // Get a reference to the configuration YAML root node.
    // If it is not available, then skip configuration node parsing.
    if (configNode_)
    {
        // Parse 'namespaces' section of configuration YAML if it exists.
        const YAML::Node namespacesNode = configNode_["namespaces"];
        if (namespacesNode)
        {
            // Check that 'namespaces' node in configuration YAML is a map.
            if (!namespacesNode.IsMap())
            {
                throw std::runtime_error(
                    "'namespaces' in configuration YAML must be a map.");
            }

            // Resolve namespace configuration entries within provided AST.
            for (const auto &it : namespacesNode)
            {
                std::string ns_str = it.first.as<std::string>();
                auto ns = chimera::util::resolveNamespace(ci, ns_str);
                if (ns)
                {
                    if (it.second.IsNull())
                    {
                        namespacesSuppressed_.insert(ns);
                    }
                    else
                    {
                        declarations_[ns] = it.second;
                        namespacesIncluded_.insert(ns);
                    }
                }
                else
                {
                    if (GetStrict())
                    {
                        throw std::runtime_error(
                            "Unable to resolve namespace: '" + ns_str + "'.");
                    }
                    else
                    {
                        std::cerr << "Warning: Skipped namespace namespace '"
                                  << ns_str
                                  << "' because it's unable to resolve "
                                  << "the namespace." << std::endl;
                    }
                }
            }
        }

        // Parse 'classes' section of configuration YAML if it exists.
        const YAML::Node classesNode = configNode_["classes"];
        if (classesNode)
        {
            // Check that 'classes' node in configuration YAML is a map.
            if (!classesNode.IsMap())
            {
                throw std::runtime_error(
                    "'classes' in configuration YAML must be a map.");
            }

            // Resolve class/struct configuration entries within provided AST.
            for (const auto it : configNode_["classes"])
            {
                std::string decl_str = it.first.as<std::string>();

                // TODO: Use better way to detect if [decl_str] is template
                // class type
                if (util::startsWith(decl_str, "template ")
                    || util::startsWith(decl_str, "template<"))
                {
                    auto decl
                        = chimera::util::resolveClassTemplate(ci, decl_str);
                    if (decl)
                    {
                        declarations_[decl] = it.second;
                        continue;
                    }
                }
                else
                {
                    auto decl = chimera::util::resolveRecord(ci, decl_str);
                    if (decl)
                    {
                        declarations_[decl] = it.second;
                        continue;
                    }
                }

                if (GetStrict())
                {
                    throw std::runtime_error(
                        "Unable to resolve class declaration: '" + decl_str
                        + "'");
                }
                else
                {
                    std::cerr
                        << "Warning: Skipped the configuration for class '"
                        << decl_str << "' becuase it's "
                        << "unable to resolve the class declaration."
                        << std::endl;
                }
            }
        }

        // Parse 'functions' section of configuration YAML if it exists.
        const YAML::Node functionsNode = configNode_["functions"];
        if (functionsNode)
        {
            // Check that 'functions' node in configuration YAML is a map.
            if (!functionsNode.IsMap())
            {
                throw std::runtime_error(
                    "'functions' in configuration YAML must be a map.");
            }

            // Resolve function configuration entries within provided AST.
            for (const auto &it : functionsNode)
            {
                std::string decl_str = it.first.as<std::string>();
                auto decl = chimera::util::resolveDeclaration(ci, decl_str);
                if (decl)
                {
                    declarations_[decl] = it.second;
                }
                else
                {
                    if (GetStrict())
                    {
                        throw std::runtime_error(
                            "Unable to resolve function declaration: '"
                            + decl_str + "'");
                    }
                    else
                    {
                        std::cerr
                            << "Warning: Skipped the configuration for "
                            << "function '" << decl_str << "' becuase it's "
                            << "unable to resolve the function declaration."
                            << std::endl;
                    }
                }
            }
        }

        // Parse 'types' section of configuration YAML if it exists.
        const YAML::Node typesNode = configNode_["types"];
        if (typesNode)
        {
            // Check that 'types' node in configuration YAML is a map.
            if (!typesNode.IsMap())
            {
                throw std::runtime_error(
                    "'types' in configuration YAML must be a map.");
            }

            // Resolve type configuration entries within provided AST.
            for (const auto &it : typesNode)
            {
                std::string type_str = it.first.as<std::string>();
                auto type = chimera::util::resolveType(ci, type_str);
                if (type.getTypePtrOrNull())
                {
                    types_.emplace_back(std::make_pair(type, it.second));
                }
                else
                {
                    if (GetStrict())
                    {
                        throw std::runtime_error("Unable to resolve type: '"
                                                 + type_str + "'");
                    }
                    else
                    {
                        std::cerr << "Warning: Skipped the configuration for "
                                  << "type '" << type_str << "' becuase it's "
                                  << "unable to resolve the type." << std::endl;
                        continue;
                    }
                }
            }
        }

        // Parse 'binding' section of configuration YAML if it exists.
        const YAML::Node bindingNode = configNode_["binding"];
        if (bindingNode)
        {
            // Check that 'binding' node in configuration YAML is a scalar.
            if (!bindingNode.IsScalar())
            {
                throw std::runtime_error(
                    "'binding' in configuration YAML must be a scalar.");
            }
            config_binding_name = bindingNode.as<std::string>();
        }
    }

    // Set the binding name from one of the following sources in order of
    // priority: 1) CLI '--binding' setting 2) YAML configuration setting 3)
    // chimera::binding::DEFAULT_NAME
    if (!parent_.bindingName_.empty())
    {
        binding_name_ = parent_.bindingName_;
    }
    else if (!config_binding_name.empty())
    {
        binding_name_ = config_binding_name;
    }
    else
    {
        binding_name_ = chimera::binding::DEFAULT_NAME;
    }

    // Resolve the base binding definition from the specified binding name.
    const auto bindingIt = chimera::binding::DEFINITIONS.find(binding_name_);
    if (bindingIt == chimera::binding::DEFINITIONS.end())
    {
        throw std::runtime_error(
            "Unable to resolve binding definition: " + binding_name_ + "'");
    }
    bindingDefinition_ = bindingIt->second;

    // Override individual templates if specified in the configuration.
    SetBindingDefinitions("class", bindingDefinition_.class_h,
                          bindingDefinition_.class_cpp);
    SetBindingDefinitions("enum", bindingDefinition_.enum_h,
                          bindingDefinition_.enum_cpp);
    SetBindingDefinitions("function", bindingDefinition_.function_h,
                          bindingDefinition_.function_cpp);
    SetBindingDefinitions("module", bindingDefinition_.module_h,
                          bindingDefinition_.module_cpp);
    SetBindingDefinitions("variable", bindingDefinition_.variable_h,
                          bindingDefinition_.variable_cpp);
    SetBindingDefinitions("typedef", bindingDefinition_.typedef_h,
                          bindingDefinition_.typedef_cpp);

    // Set custom escape function that disables HTML escaping on mstch output.
    //
    // This is not desirable in chimera because many C++ types include
    // characters that can be accidentally escaped, such as `<>` and `&`.
    //
    // See: https://github.com/no1msd/mstch#custom-escape-function
    //
    ::mstch::config::escape
        = [](const std::string &str) -> std::string { return str; };
}

bool chimera::CompiledConfiguration::GetStrict() const
{
    return strict_;
}

void chimera::CompiledConfiguration::AddTraversedNamespace(
    const clang::NamespaceDecl *decl)
{
    // Skip namespaces that are defined as null in the configuration.
    for (const auto &it : GetNamespacesSuppressed())
    {
        if (decl && it->Encloses(decl))
            return;
    }

    // We need to preserve the order of the traversed namespace declarations,
    // as the ASTConsumer traverses them in a hierarchical order.
    //
    // We first use a set to de-duplicate the namespaces using their canonical
    // decl pointers, then insert the renderable proxy of new namespaces into
    // a vector which will preserve their order during Render().
    const auto result
        = binding_namespace_decls_.insert(decl->getCanonicalDecl());
    if (result.second)
    {
        binding_namespaces_.push_back(
            std::make_shared<chimera::mstch::Namespace>(*this, decl));
    }
}

const std::set<const clang::NamespaceDecl *>
    &chimera::CompiledConfiguration::GetNamespacesIncluded() const
{
    return namespacesIncluded_;
}

const std::set<const clang::NamespaceDecl *>
    &chimera::CompiledConfiguration::GetNamespacesSuppressed() const
{
    return namespacesSuppressed_;
}

const YAML::Node &chimera::CompiledConfiguration::GetDeclaration(
    const clang::Decl *decl) const
{
    const auto d = declarations_.find(decl->getCanonicalDecl());
    return d != declarations_.end() ? d->second : emptyNode_;
}

const YAML::Node &chimera::CompiledConfiguration::GetType(
    const clang::QualType type) const
{
    const auto canonical_type = type.getCanonicalType();
    for (const auto &entry : types_)
    {
        if (entry.first == canonical_type)
            return entry.second;
    }
    return emptyNode_;
}

clang::CompilerInstance *chimera::CompiledConfiguration::GetCompilerInstance()
    const
{
    return ci_;
}

clang::ASTContext &chimera::CompiledConfiguration::GetContext() const
{
    return ci_->getASTContext();
}

const std::string &chimera::CompiledConfiguration::GetBindingName() const
{
    return binding_name_;
}

bool chimera::CompiledConfiguration::IsEnclosed(const clang::Decl *decl) const
{
    // Skip namespaces that are defined as null in the configuration.
    for (const auto &it : GetNamespacesSuppressed())
    {
        if (decl->getDeclContext() && it->Encloses(decl->getDeclContext()))
            return false;
    }

    // Filter over the namespaces and only traverse ones that are enclosed
    // by one of the configuration namespaces.
    for (const auto &it : GetNamespacesIncluded())
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
    if (!IsEnclosed(decl))
        return true;

    const auto config = GetDeclaration(decl);

    // If the declaration is directly suppressed, report this.
    if (config.IsNull())
        return true;

    // If the (canonical) declaration is specialized template class, check if
    // the template class is suppressed.
    auto canonical_decl = decl->getCanonicalDecl();
    if (auto specialized_templ_decl
        = dyn_cast<ClassTemplateSpecializationDecl>(canonical_decl))
    {
        auto templ_decl = specialized_templ_decl->getSpecializedTemplate();
        const auto config = GetDeclaration(templ_decl);
        if (config.IsNull())
            return true;
    }

    // Functions can be suppressed if they return a suppressed type or take
    // suppressed parameter types.
    if (isa<FunctionDecl>(decl))
    {
        const FunctionDecl *function_decl = cast<FunctionDecl>(decl);

        // Check if they return a suppressed type.
        if (!config["return_value_policy"])
        {
            const QualType return_qual_type
                = chimera::util::getFullyQualifiedType(
                    GetContext(), function_decl->getReturnType());
            if (IsSuppressed(return_qual_type))
                return true;
        }

        // Check if they take suppressed parameter types.
        for (auto i = 0u; i < function_decl->getNumParams(); ++i)
        {
            const ParmVarDecl *param_decl = function_decl->getParamDecl(i);
            QualType param_type = param_decl->getType();

            if (param_type->isReferenceType())
                param_type = param_type.getNonReferenceType();

            if (auto elaborated_type = dyn_cast<ElaboratedType>(param_type))
                param_type = elaborated_type->desugar();

            if (auto param_tag_type = dyn_cast<TagType>(param_type))
            {
                TagDecl *param_tag_decl
                    = cast<TagDecl>(param_tag_type->getDecl());
                if (GetDeclaration(param_tag_decl).IsNull())
                    return true;
            }

            if (auto template_specialization_type
                = dyn_cast<TemplateSpecializationType>(param_type))
            {
                auto template_decl
                    = template_specialization_type->getTemplateName()
                          .getAsTemplateDecl();
                if (GetDeclaration(template_decl).IsNull())
                    return true;
            }
        }
    }
    // Fields can be suppressed if they represent a suppressed type.
    else if (isa<FieldDecl>(decl) && !config["return_value_policy"])
    {
        const FieldDecl *field_decl = cast<FieldDecl>(decl);
        const QualType value_qual_type = chimera::util::getFullyQualifiedType(
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
    {
        throw std::invalid_argument(
            "Unable to parse expected scalar or file source.");
    }

    // If the node type tag is "!file" then load the contents of a file.
    if (node.Tag() == "!file")
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
            std::stringstream ss;
            ss << "Failed to open source '" << source_path
               << "': " << strerror(errno);
            throw std::runtime_error(ss.str());
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

bool chimera::CompiledConfiguration::Render(
    const std::string &key, const std::string &header_view,
    const std::string &source_view,
    const std::shared_ptr<::mstch::object> &context)
{
    // Get the mangled name property if it exists.
    if (!context->has("mangled_name"))
    {
        std::cerr << "Cannot serialize template with no mangled name."
                  << std::endl;
        return false;
    }
    const std::string mangled_name
        = ::mstch::render("{{mangled_name}}", context);

    // Create collections for the ordered sets of sources.
    ::mstch::array binding_sources(parent_.inputSourcePaths_.begin(),
                                   parent_.inputSourcePaths_.end());

    // Create a top-level context that contains the extracted information
    // about this particular binding component.
    ::mstch::map full_context{{key, context}, {"sources", binding_sources}};

    // Resolve customizable snippets that will be inserted into the file
    // from the configuration file's "template::file" entry.
    if (bindingNode_)
    {
        chimera::util::extendWithYAMLNode(
            full_context, bindingNode_["file"], false,
            std::bind(&chimera::CompiledConfiguration::Lookup, this,
                      std::placeholders::_1));
    }

    if (!Render(mangled_name, header_view, "h", context, full_context))
        return false;

    if (!Render(mangled_name, source_view, "cpp", context, full_context))
        return false;

    // Record this binding name for use at the top-level.
    binding_names_.push_back(mangled_name);
    return true;
}

bool chimera::CompiledConfiguration::Render(
    const std::string &mangled_name, const std::string &view,
    const std::string &extension,
    const std::shared_ptr<::mstch::object> &context,
    const ::mstch::map &full_context)
{
    if (view == chimera::util::FLAG_NO_RENDER)
        return true;

    // Create and sanitize path and filename of top-level source file.
    // Because we may compress the filename to fit OS character limits,
    // we generate the full path, then split the filename from it.
    const std::string binding_path = sanitizePath(
        parent_.GetOutputPath() + "/" + mangled_name + "." + extension);
    size_t path_index = binding_path.find_last_of("/");
    const std::string binding_filename
        = (path_index == std::string::npos)
              ? ""
              : binding_path.substr(path_index + 1);

    // Create an output file depending on the provided parameters.
    auto stream = ci_->createOutputFile(
        binding_path,
        false, // Open the file in binary mode
        false, // Register with llvm::sys::RemoveFileOnSignal
        "",    // The derived basename (shouldn't be used)
        "",    // The extension to use for derived name (shouldn't be used)
        false, // Use a temporary file that should be renamed
        false  // Create missing directories in the output path
    );

    // If file creation failed, report the error and fail immediately.
    if (!stream)
    {
        std::stringstream ss;

        if (context)
        {
            ss << "Failed to create output file '" << binding_path << "' for '"
               << ::mstch::render("{{name}}", context) << "'.";
        }
        else
        {
            ss << "Failed to create top-level output file '" << binding_path
               << "'";
        }
        throw std::runtime_error(ss.str());
    }

    // Render the mstch template to the given output file.
    *stream << ::mstch::render(view, full_context);
    std::cout << binding_filename << std::endl;

    return true;
}

bool chimera::CompiledConfiguration::Render(
    const std::shared_ptr<chimera::mstch::CXXRecord> context)
{
    return Render("class", bindingDefinition_.class_h,
                  bindingDefinition_.class_cpp, context);
}

bool chimera::CompiledConfiguration::Render(
    const std::shared_ptr<chimera::mstch::Enum> context)
{
    return Render("enum", bindingDefinition_.enum_h,
                  bindingDefinition_.enum_cpp, context);
}

bool chimera::CompiledConfiguration::Render(
    const std::shared_ptr<chimera::mstch::Function> context)
{
    return Render("function", bindingDefinition_.function_h,
                  bindingDefinition_.function_cpp, context);
}

bool chimera::CompiledConfiguration::Render(
    const std::shared_ptr<chimera::mstch::Variable> context)
{
    return Render("variable", bindingDefinition_.variable_h,
                  bindingDefinition_.variable_cpp, context);
}

bool chimera::CompiledConfiguration::Render(
    const std::shared_ptr<chimera::mstch::Typedef> context)
{
    return Render("typedef", bindingDefinition_.typedef_h,
                  bindingDefinition_.typedef_cpp, context);
}

bool chimera::CompiledConfiguration::Render(
    const std::shared_ptr<chimera::mstch::BuiltinTypedef> context)
{
    return Render("typedef", bindingDefinition_.typedef_h,
                  bindingDefinition_.typedef_cpp, context);
}

void chimera::CompiledConfiguration::Render()
{
    // Create collections for the ordered sets of bindings, sources,
    // and namespaces.
    ::mstch::array binding_names(binding_names_.begin(), binding_names_.end());
    ::mstch::array binding_namespaces(binding_namespaces_.begin(),
                                      binding_namespaces_.end());
    ::mstch::array binding_sources(parent_.inputSourcePaths_.begin(),
                                   parent_.inputSourcePaths_.end());

    // Create a top-level context that contains the extracted information
    // about the module.
    ::mstch::map full_context{
        {"module",
         ::mstch::map{{"name", parent_.GetOutputModuleName()},
                      {"bindings", binding_names},
                      {"sources", binding_sources},
                      // Note: binding namespaces will be lexically ordered.
                      {"namespaces", binding_namespaces}}}};

    // Resolve customizable snippets that will be inserted into the file
    // from the configuration file's "template::module" entry.
    if (bindingNode_)
    {
        chimera::util::extendWithYAMLNode(
            full_context, bindingNode_["module"], false,
            std::bind(&chimera::CompiledConfiguration::Lookup, this,
                      std::placeholders::_1));
    }

    // Render the mstch template to the given output file.
    const auto &filename = parent_.GetOutputModuleName();
    Render(filename, bindingDefinition_.module_h, "h", nullptr, full_context);
    Render(filename, bindingDefinition_.module_cpp, "cpp", nullptr,
           full_context);
}

void chimera::CompiledConfiguration::SetBindingDefinitions(
    const std::string &key, std::string &header_def, std::string &source_def)
{
    using chimera::util::lookupYAMLNode;

    if (const auto node = lookupYAMLNode(bindingNode_, key))
    {
        if (node.IsNull())
        {
            header_def = chimera::util::FLAG_NO_RENDER;
            source_def = chimera::util::FLAG_NO_RENDER;
        }
        else if (node.IsScalar())
        {
            header_def = chimera::util::FLAG_NO_RENDER;
            source_def = Lookup(node);
        }
        else
        {
            if (const auto header_node = lookupYAMLNode(node, "header"))
                header_def = Lookup(header_node);
            if (const auto source_node = lookupYAMLNode(node, "source"))
                source_def = Lookup(source_node);
        }
    }
}
