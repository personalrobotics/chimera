#include "chimera/consumer.h"

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <iostream>

using namespace clang;

chimera::Consumer::Consumer(CompilerInstance *CI)
: visitor_(CI)
{ 
    // Do nothing.
}

void chimera::Consumer::HandleTranslationUnit(ASTContext &Context)
{
    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file.
    visitor_.TraverseDecl(Context.getTranslationUnitDecl());
}

