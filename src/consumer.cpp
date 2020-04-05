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

    std::unique_ptr<chimera::CompiledConfiguration> compiled_config
        = config.Process(ci_);
    chimera::Visitor visitor(ci_, *compiled_config);

    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file.
    visitor.TraverseDecl(context.getTranslationUnitDecl());

    // Render the top-level mstch template
    compiled_config->Render();
}
