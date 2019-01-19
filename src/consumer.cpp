#include "chimera/consumer.h"
#include "chimera/configuration.h"
#include "chimera/visitor.h"

#include <iostream>

using namespace clang;

chimera::Consumer::Consumer(CompilerInstance *ci) : ci_(ci)
{
    // Do nothing.
}

void chimera::Consumer::HandleTranslationUnit(ASTContext &context)
{
    // Use the current translation unit to resolve the YAML configuration.
    chimera::Configuration &config = chimera::Configuration::GetInstance();
    chimera::Visitor visitor(ci_, config.Process(ci_));

    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file.
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}
