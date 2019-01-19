#include "chimera/frontend_action.h"
#include "chimera/consumer.h"

#include <clang/Parse/Parser.h>

using namespace clang;

std::unique_ptr<clang::ASTConsumer> chimera::FrontendAction::CreateASTConsumer(
    CompilerInstance &CI, StringRef file)
{
    CI.getPreprocessor().getDiagnostics().setIgnoreAllWarnings(true);
    return std::unique_ptr<chimera::Consumer>(new chimera::Consumer(&CI));
}
