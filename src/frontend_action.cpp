#include "chimera/frontend_action.h"
#include "chimera/consumer.h"

using namespace clang;

std::unique_ptr<clang::ASTConsumer> 
chimera::FrontendAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) 
{
    return std::unique_ptr<chimera::Consumer>(new chimera::Consumer(&CI));
}
