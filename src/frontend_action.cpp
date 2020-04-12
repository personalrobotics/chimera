#include "chimera/frontend_action.h"
#include "chimera/consumer.h"

#include <clang/Parse/Parser.h>

using namespace clang;

chimera::FrontendAction::FrontendAction(const chimera::Configuration &config)
  : config_(config)
{
    // Do nothing.
}

std::unique_ptr<clang::ASTConsumer> chimera::FrontendAction::CreateASTConsumer(
    CompilerInstance &CI, StringRef /*file*/)
{
    // For some unknown reason, this is called twice, which should be once.
    // As a workaround, we check if this called first, otherwise return nullptr.
    static bool is_first = true;
    if (!is_first)
        return nullptr;

    is_first = false;

    CI.getPreprocessor().getDiagnostics().setIgnoreAllWarnings(true);
    return std::unique_ptr<chimera::Consumer>(
        new chimera::Consumer(&CI, config_));
}

// Create a custom action factory that forwards the ChimeraConfiguration.
chimera::ChimeraFrontendActionFactory::ChimeraFrontendActionFactory(
    const chimera::Configuration &config)
  : config_(config)
{
    // Do nothing.
}

// Between Clang 9 and Clang 10, the return value for
// FrontendActionFactory::create() changed from raw pointer to std::unique_ptr.
#if LLVM_VERSION_AT_LEAST(10, 0, 0)
std::unique_ptr<FrontendAction> chimera::ChimeraFrontendActionFactory::create()
{
    return std::unique_ptr<FrontendAction>(
        new chimera::FrontendAction(config_));
}
#else
FrontendAction *chimera::ChimeraFrontendActionFactory::create()
{
    return new chimera::FrontendAction(config_);
}
#endif

std::unique_ptr<tooling::FrontendActionFactory>
chimera::newFrontendActionFactory(const chimera::Configuration &config)
{
    return std::unique_ptr<tooling::FrontendActionFactory>(
        new chimera::ChimeraFrontendActionFactory(config));
}
