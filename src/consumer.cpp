#include "chimera/consumer.h"
#include "chimera/configuration.h"

#include <iostream>

using namespace clang;

chimera::Consumer::Consumer(CompilerInstance *ci)
: ci_(ci)
, visitor_(ci)
{ 
    // Do nothing.
}

void chimera::Consumer::HandleTranslationUnit(ASTContext &context)
{
    // Use the current translation unit to resolve the YAML configuration.
    chimera::Configuration &config = chimera::Configuration::GetInstance();
    config.Process(ci_);

    auto namespaces = config.GetNamespaces();
    for(auto it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        std::string decl_str = (*it)->getNameAsString();
        std::cout << "NS: " << decl_str << std::endl;
    }

    // We can use ASTContext to get the TranslationUnitDecl, which is
    // a single Decl that collectively represents the entire source file.
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
}

