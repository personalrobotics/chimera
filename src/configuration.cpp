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

template <class Iterator>
std::string join(
    Iterator begin, Iterator end, const std::string &delimiter)
{
  if (begin == end)
    return "";

  std::stringstream stream;

  for (Iterator it = begin; it != end; ++it)
    stream << delimiter << *it;

  return stream.str().substr(delimiter.size());
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

chimera::StreamUniquePtr
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
    std::string mangled_name;
    llvm::raw_string_ostream mangled_name_stream(mangled_name);
    mangler_->mangleName(named_decl, mangled_name_stream);
    mangled_name = mangled_name_stream.str();

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

    // Resolve customizable snippets that will be inserted into the file.
    const YAML::Node &template_config = parent_.GetRoot()["template"]["file"];
    std::string header_snippet, postinclude_snippet, footer_snippet;
    getSnippet(template_config["header"],
               parent_.GetConfigFilename(), header_snippet);
    getSnippet(template_config["postinclude"],
               parent_.GetConfigFilename(), postinclude_snippet);
    getSnippet(template_config["footer"],
               parent_.GetConfigFilename(), footer_snippet);

    // Create a stream wrapper to write header and footer of file.
    std::cout << binding_filename << std::endl;
    return StreamUniquePtr(
      new chimera::Stream(
        stream, binding_prototype, includes_,
        header_snippet, postinclude_snippet, footer_snippet),
      [this, binding_prototype, mangled_name](chimera::Stream *stream)
      {
        // Add this function to the top-level binding source file. This must
        // occur when the stream is closed to avoid generating references to
        // namespaces that do not yet exist.
        *binding_ << "  " << binding_prototype << ";\n";
        *binding_ << "  " << mangled_name << "();\n\n";

        delete stream;
      }
    );
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

bool chimera::CompiledConfiguration::DumpNamespace(
    const clang::NestedNameSpecifier *nns)
{
    if (nns->getKind() != clang::NestedNameSpecifier::Namespace)
      throw std::runtime_error("NestedNameSpecifier is not a Namespace.");

    const clang::NamespaceDecl *canonical_decl
      = nns->getAsNamespace()->getCanonicalDecl();

    // Check if a Python module was already created for this namespace.
    if (dumped_namespaces_.count(canonical_decl))
        return false;

    // Build a list of NestedNameeSpecifiers starting at the root.
    std::vector<const NestedNameSpecifier *> namespaces;
    while (nns)
    {
      namespaces.push_back(nns);
      nns = nns->getPrefix();
    }

    std::reverse(std::begin(namespaces), std::end(namespaces));

    std::vector<std::string> module_list;
    for (const NestedNameSpecifier *nns : namespaces)
    {
      if (nns->getKind() != clang::NestedNameSpecifier::Namespace)
        throw std::runtime_error(
          "Prefix of NestedNameSpecifier is not all Namespaces.");

      const NamespaceDecl *namespace_decl
        = nns->getAsNamespace()->getCanonicalDecl();

      // Skip root namespaces.
      if (namespaces_.count(namespace_decl))
        continue;

      module_list.push_back(namespace_decl->getNameAsString());
    }

    if (module_list.empty())
      return false;

    // Generate the Python module.
    const size_t isubmodule = dumped_namespaces_.size();
    const std::string module_path = parent_.GetOutputModuleName() + "."
        + join(std::begin(module_list), std::end(module_list), ".");

    *binding_
        << "  ::boost::python::scope()";

    for (const std::string &submodule_name : module_list)
      *binding_ << ".attr(\"" << submodule_name << "\")";

    *binding_
        << " = "
           "::boost::python::object("
           // TODO: What does handle<> do?
           "::boost::python::handle<>("
           // TODO: Should this be borrowed() or incref()?
           "::boost::python::borrowed("
           "::PyImport_AddModule(\""
        << module_path << "\"))));\n\n";

    dumped_namespaces_.insert(canonical_decl);
    return true;
}
