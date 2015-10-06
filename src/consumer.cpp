#include "chimera/consumer.h"
#include "chimera/configuration.h"
#include "chimera/visitor.h"

#include <iostream>

using namespace clang;

chimera::Consumer::Consumer(CompilerInstance *ci, StringRef file)
: ci_(ci), file_(file)
{ 
    // Do nothing.
}

void chimera::Consumer::HandleTranslationUnit(ASTContext &context)
{
    // Use the current translation unit to resolve the YAML configuration.
    chimera::Configuration &config = chimera::Configuration::GetInstance();
    chimera::Visitor visitor(ci_, config.Process(ci_, file_));

    // TODO: Remove this debug print.
    std::cout << "\n\n---\n\n";

    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file.
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}
