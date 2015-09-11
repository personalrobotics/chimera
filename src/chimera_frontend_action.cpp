#include "chimera/chimera_frontend_action.h"
#include "chimera/chimera_consumer.h"

using namespace chimera;
using namespace clang;

std::unique_ptr<clang::ASTConsumer> 
ChimeraFrontendAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) 
{
    return std::unique_ptr<ChimeraConsumer>(new ChimeraConsumer(&CI));
}


