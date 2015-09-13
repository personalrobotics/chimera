#include "chimera/util.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/Parser.h>
#include <clang/Sema/Sema.h>
#include <iostream>

using namespace clang;

const NamedDecl*
chimera::util::resolveDeclaration(CompilerInstance *ci,
                                  const llvm::StringRef declStr)
{
    // Immediately return if passed an empty string.
    if (declStr.empty())
        return NULL;

    // Create a new parser to handle this type parsing.
    Preprocessor &preprocessor = ci->getPreprocessor();
    Sema &sema = ci->getSema();
    Parser parser(preprocessor, sema, /* SkipFunctionBodies = */ false);

    // Set up the preprocessor to only care about incrementally handling type.
    preprocessor.getDiagnostics().setSuppressAllDiagnostics(true);
    preprocessor.enableIncrementalProcessing();
    const_cast<LangOptions&>(preprocessor.getLangOpts()).SpellChecking = 0;

    // Put the type string into a buffer and run it through the preprocessor.
    FileID fid = sema.getSourceManager().createFileID(
        llvm::MemoryBuffer::getMemBufferCopy(declStr.str() + ";",
                                             "chimera.util.findtype"));
    preprocessor.EnterSourceFile(fid, /* DirLookup = */ 0, SourceLocation());
    parser.Initialize();
    
    // Try parsing the type name.
    Parser::DeclGroupPtrTy ADecl;
    while (!parser.ParseTopLevelDecl(ADecl))
    {
        // A DeclGroupRef may have multiple Decls, so we iterate through each one.
        // TODO: We probably shouldn't actually iterate here.
        if (ADecl)
        {
            std::cout << "PARSED " << declStr.str() << std::endl;
            DeclGroupRef DG = ADecl.get();
            for (auto i = DG.begin(), e = DG.end(); i != e; ++i)
            {
                Decl *D = (*i)->getCanonicalDecl();
                D->dumpColor(); // TODO: remove debugging.

                return (isa<NamedDecl>(D)) ? cast<NamedDecl>(D) : NULL;
            }
        }
    }

    std::cerr << "FAILED TO PARSE: " << declStr.str() << std::endl;
    return NULL;
}

const NamespaceDecl*
chimera::util::resolveNamespace(clang::CompilerInstance *ci,
                                const llvm::StringRef nsStr)
{
    auto D = chimera::util::resolveDeclaration(ci, "namespace " + nsStr.str() + " {}");
    return (isa<NamespaceDecl>(D)) ? cast<NamespaceDecl>(D) : NULL;
}
