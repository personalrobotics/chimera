#include "chimera/consumer.h"
#include "chimera/visitor.h"

#include <iostream>

using namespace clang;

chimera::Consumer::Consumer(CompilerInstance *ci,
                            const chimera::Configuration &config)
  : ci_(ci), config_(config)
{
    // Do nothing.
}

void chimera::Consumer::HandleTranslationUnit(ASTContext &context)
{
    // Use the current translation unit to resolve the YAML configuration.
    std::unique_ptr<chimera::CompiledConfiguration> compiled_config
        = config_.Process(ci_);
    chimera::Visitor visitor(ci_, *compiled_config);

    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file.
    visitor.TraverseDecl(context.getTranslationUnitDecl());

    // Render the top-level mstch template
    compiled_config->Render();
}
