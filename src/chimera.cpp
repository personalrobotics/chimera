/**
 * Chimera - a tool to convert c++ headers into Boost.Python bindings.
 */
#include "chimera/chimera.h"
#include "chimera/configuration.h"
#include "chimera/frontend_action.h"

#include <iostream>
#include <memory>
#include <string>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>

#define STR_DETAIL(x) #x
#define STR(x) STR_DETAIL(x)

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

namespace chimera
{

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory ChimeraCategory("Chimera options");

// Option for specifying binding type by name.
static cl::opt<std::string> BindingName(
    "b", cl::cat(ChimeraCategory), cl::desc("Specify binding definition name"),
    cl::value_desc("binding"));

// Option for specifying output binding filename.
static cl::opt<std::string> OutputPath(
    "o", cl::cat(ChimeraCategory),
    cl::desc("Specify output bindings directory"), cl::value_desc("directory"));

// Option for specifying output binding filename.
static cl::opt<std::string> OutputModuleName(
    "m", cl::cat(ChimeraCategory),
    cl::desc("Specify output top-level module name"),
    cl::value_desc("modulename"));

// Option for specifying top-level binding namespaces.
static cl::list<std::string> NamespaceNames(
    "n", cl::cat(ChimeraCategory),
    cl::desc("Specify one or more top-level namespaces that will be bound"),
    cl::value_desc("namespace"));

// Option for specifying YAML configuration filename.
static cl::opt<std::string> ConfigFilename(
    "c", cl::cat(ChimeraCategory),
    cl::desc("Specify YAML configuration filename"),
    cl::value_desc("filename"));

// Option for switching from C++ to C source.
static cl::opt<bool> UseCMode(
    "use-c", cl::cat(ChimeraCategory),
    cl::desc("Parse input files as C instead of C++"));

// Option for preventing auto-extraction of comments from source files.
static cl::opt<bool> SuppressDocs(
    "no-docs", cl::cat(ChimeraCategory),
    cl::desc("Suppress the extraction of documentation from C++ comments"));

// Option for preventing binding sources from being auto-forwarded in generated
// bindings.
static cl::opt<bool> SuppressSources(
    "no-default-sources", cl::cat(ChimeraCategory),
    cl::desc("Suppress the forwarding of source file paths to binding"));

// Option for treating unresolvable configuration as errors.
static cl::opt<bool> Strict(
    "strict", cl::cat(ChimeraCategory),
    cl::desc("Treat unresolvable configuration as errors"));

// Add a footer to the help text.
static cl::extrahelp MoreHelp(
    "\n"
    "Chimera is a tool to convert C++ headers into Boost.Python bindings.\n"
    "\n");

int run(int argc, const char **argv)
{
    // Print custom output for `--version` option
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--version") == 0)
        {
            std::cout << "Chimera " << CHIMERA_MAJOR_VERSION << "."
                      << CHIMERA_MINOR_VERSION << "." << CHIMERA_PATCH_VERSION
                      << "\n\n";
            exit(0);
        }
    }

    // Create parser that handles clang options.
    CommonOptionsParser OptionsParser(argc, argv, ChimeraCategory);

    // Create a new configuration that will be used for the run.
    chimera::Configuration Config;

    // Parse the YAML configuration file if it exists, otherwise initialize it
    // to an empty node.
    if (!ConfigFilename.empty())
        Config.LoadFile(ConfigFilename);

    // If a binding definition was specified, set configuration to use it.
    if (!BindingName.empty())
        Config.SetBindingName(BindingName);

    // If an output path was specified, set configuration to use it.
    if (!OutputPath.empty())
        Config.SetOutputPath(OutputPath);

    // If a top-level binding file was specified, set configuration to use it.
    if (!OutputModuleName.empty())
        Config.SetOutputModuleName(OutputModuleName);

    // Add top-level namespaces to the configuration.
    if (NamespaceNames.size())
        for (const std::string &name : NamespaceNames)
            Config.AddInputNamespaceName(name);

    // Add compilation source paths to the configuration.
    // These will be made available to templates.
    if (!SuppressSources)
        for (const std::string &path : OptionsParser.getSourcePathList())
            Config.AddSourcePath(path);

    // If strict option is on, treats unresolvable configuration as errors.
    if (Strict)
        Config.SetStrict(true);

    // Create tool that uses the command-line options.
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    // Add or suppress clang documentation flag as specified.
    Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
        SuppressDocs ? "-Wno-documentation" : "-Wdocumentation",
        ArgumentInsertPosition::BEGIN));

    // Add the appropriate C/C++ language flag.
    Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
        UseCMode ? "-xc" : "-xc++", ArgumentInsertPosition::BEGIN));

    // Run the instantiated tool on the Chimera frontend.
    return Tool.run(chimera::newFrontendActionFactory(Config).get());
}

} // namespace chimera
