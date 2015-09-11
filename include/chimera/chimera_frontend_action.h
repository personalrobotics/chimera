#ifndef __CHIMERA_CHIMERA_FRONTEND_ACTION_H__
#define __CHIMERA_CHIMERA_FRONTEND_ACTION_H__

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Frontend/FrontendActions.h"

namespace chimera
{

/**
 * Front-end that runs the Chimera AST consumer on the provided source.
 */
class ChimeraFrontendAction : public clang::ASTFrontendAction {
public:
    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef file);
};

} // namespace chimera

#endif // __CHIMERA_CHIMERA_FRONTEND_ACTION_H__