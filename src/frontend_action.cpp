#include "chimera/frontend_action.h"
#include "chimera/consumer.h"

#include <clang/Parse/Parser.h>

using namespace clang;

std::unique_ptr<clang::ASTConsumer> chimera::FrontendAction::CreateASTConsumer(
    CompilerInstance &CI, StringRef file)
{
    // For some unknown reason, this is called twice, which should be once.
    // As a workaround, we check if this called first, otherwise return nullptr.
    static bool is_first = true;
    if (!is_first)
        return nullptr;

    is_first = false;

    CI.getPreprocessor().getDiagnostics().setIgnoreAllWarnings(true);
    return std::unique_ptr<chimera::Consumer>(new chimera::Consumer(&CI));
}
