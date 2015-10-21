/**
 * Chimera - a tool to convert c++ headers into Boost.Python bindings.
 */
#include "chimera/configuration.h"
#include "chimera/frontend_action.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <memory>
#include <string>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory ChimeraCategory("Chimera options");

// Option for specifying output binding filename.
static cl::opt<std::string> OutputPath(
    "o", cl::cat(ChimeraCategory),
    cl::desc("Specify output C++ binding directory"),
    cl::value_desc("directory"));

// Option for specifying YAML configuration filename.
static cl::opt<std::string> ConfigFilename(
    "c", cl::cat(ChimeraCategory),
    cl::desc("Specify YAML configuration filename"),
    cl::value_desc("filename"));

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// Add a footer to the help text.
static cl::extrahelp MoreHelp(
    "\n"
    "Chimera is a tool to convert C++ headers into Boost.Python bindings.\n"
    "\n"
);

int main(int argc, const char **argv)
{
    // Create parser that handles clang options.
    CommonOptionsParser OptionsParser(argc, argv, ChimeraCategory);

    // Parse the YAML configuration file if it exists, otherwise initialize it
    // to an empty node.
    if (!ConfigFilename.empty())
        chimera::Configuration::GetInstance().LoadFile(ConfigFilename);

    // If an output path was specified, initialize configuration to use it.
    if (!OutputPath.empty())
        chimera::Configuration::GetInstance().SetOutputPath(OutputPath);

    // Create tool that uses the command-line options.
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    // Run the instantiated tool on the Chimera frontend.
    return Tool.run(newFrontendActionFactory<chimera::FrontendAction>().get());
}
