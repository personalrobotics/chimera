#ifndef __CHIMERA_CONSUMER_H__
#define __CHIMERA_CONSUMER_H__

#include "visitor.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{

class Consumer : public clang::ASTConsumer
{
public:
    // Override the constructor in order to pass CI.
    explicit Consumer(clang::CompilerInstance *CI);

    // Override this to call our ChimeraVisitor on the entire source file.
    virtual void HandleTranslationUnit(clang::ASTContext &Context);

private:
    Visitor visitor_;
};

} // namespace chimera

#endif // __CHIMERA_CONSUMER_H__