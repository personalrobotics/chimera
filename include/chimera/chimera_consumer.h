#ifndef __CHIMERA_CHIMERA_CONSUMER_H__
#define __CHIMERA_CHIMERA_CONSUMER_H__

#include "chimera_visitor.h"
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{

class ChimeraConsumer : public clang::ASTConsumer
{
public:
    // Override the constructor in order to pass CI.
    explicit ChimeraConsumer(clang::CompilerInstance *CI);

    // Override this to call our ChimeraVisitor on the entire source file.
    virtual void HandleTranslationUnit(clang::ASTContext &Context);

    // Override this to call ChimeraVisitor on each top-level Decl.
    virtual bool HandleTopLevelDecl(clang::DeclGroupRef DG);

private:
    ChimeraVisitor *visitor;
};

} // namespace chimera

#endif // __CHIMERA_CHIMERA_CONSUMER_H__