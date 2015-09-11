#include "chimera/consumer.h"

using namespace clang;

chimera::Consumer::Consumer(CompilerInstance *CI)
: visitor(CI)
{ 
    // Do nothing.
}

void chimera::Consumer::HandleTranslationUnit(ASTContext &Context)
{
    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

bool chimera::Consumer::HandleTopLevelDecl(DeclGroupRef DG)
{
    // A DeclGroupRef may have multiple Decls, so we iterate through each one.
    for (auto i = DG.begin(), e = DG.end(); i != e; ++i)
    {
        Decl *D = *i;    
        visitor.TraverseDecl(D); // recursively visit each AST node in Decl "D"
    }
    return true;
}
