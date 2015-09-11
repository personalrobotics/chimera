#include "chimera/chimera_consumer.h"

using namespace chimera;
using namespace clang;

ChimeraConsumer::ChimeraConsumer(CompilerInstance *CI)
: visitor(new ChimeraVisitor(CI))
{ 
    // Do nothing.
}

void ChimeraConsumer::HandleTranslationUnit(ASTContext &Context)
{
    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file
    visitor->TraverseDecl(Context.getTranslationUnitDecl());
}

bool ChimeraConsumer::HandleTopLevelDecl(DeclGroupRef DG)
{
    // A DeclGroupRef may have multiple Decls, so we iterate through each one.
    for (auto i = DG.begin(), e = DG.end(); i != e; ++i)
    {
        Decl *D = *i;    
        visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
    }
    return true;
}
