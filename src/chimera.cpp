/**
 * Chimera - a tool to convert c++ headers into Boost.Python bindings.
 */
#include "chimera/configuration.h"
#include "chimera/frontend_action.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <llvm/Config/config.h>
#include <llvm/Support/CommandLine.h>

#include <memory>
#include <string>

#define STR_DETAIL(x) #x
#define STR(x) STR_DETAIL(x)

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory ChimeraCategory("Chimera options");

// Option for specifying output binding filename.
static cl::opt<std::string> OutputPath(
    "o", cl::cat(ChimeraCategory),
    cl::desc("Specify output bindings directory"),
    cl::value_desc("directory"));

// Option for specifying output binding filename.
static cl::opt<std::string> OutputFilename(
    "f", cl::cat(ChimeraCategory),
    cl::desc("Specify output top-level binding filename"),
    cl::value_desc("filename"));

// Option for specifying YAML configuration filename.
static cl::opt<std::string> ConfigFilename(
    "c", cl::cat(ChimeraCategory),
    cl::desc("Specify YAML configuration filename"),
    cl::value_desc("filename"));

// Option for switching from C++ to C source.
static cl::opt<bool> UseCMode(
    "use-c", cl::cat(ChimeraCategory),
    cl::desc("Parse input files as C instead of C++"));

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

    // If an output path was specified, set configuration to use it.
    if (!OutputPath.empty())
        chimera::Configuration::GetInstance().SetOutputPath(OutputPath);

    // If a top-level binding file was specified, set configuration to use it.
    if (!OutputFilename.empty())
        chimera::Configuration::GetInstance().SetOutputFilename(OutputFilename);

    // Create tool that uses the command-line options.
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    // Add the appropriate C/C++ language flag.
    Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
        UseCMode ? "-xc" : "-xc++", ArgumentInsertPosition::BEGIN));

    // Add a workaround for the bug in clang shipped default with Ubuntu 14.04.
    // https://bugs.launchpad.net/ubuntu/+source/llvm-defaults/+bug/1242300
    Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
        "-I/usr/lib/llvm-" STR(LLVM_VERSION_MAJOR) "." STR(LLVM_VERSION_MINOR)
        "/lib/clang/" LLVM_VERSION_STRING "/include", ArgumentInsertPosition::END));
    

    // Run the instantiated tool on the Chimera frontend.
    return Tool.run(newFrontendActionFactory<chimera::FrontendAction>().get());
}
