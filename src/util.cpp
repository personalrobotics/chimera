#include "chimera/util.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/Parser.h>
#include <clang/Sema/Sema.h>
#include <iostream>

using namespace clang;

QualType chimera::util::findType(CompilerInstance *ci, llvm::StringRef typeName)
{
    QualType qt;

    // Immediately return if passed an empty string.
    if (typeName.empty())
        return qt;

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
        llvm::MemoryBuffer::getMemBufferCopy(typeName.str() + ";",
                                             "chimera.util.findtype"));
    preprocessor.EnterSourceFile(fid, /* DirLookup = */ 0, SourceLocation());
    parser.Initialize();
    
    // Try parsing the type name.
    Parser::DeclGroupPtrTy ADecl;
    while (!parser.ParseTopLevelDecl(ADecl))
    {
        // A DeclGroupRef may have multiple Decls, so we iterate through each one.
        if (ADecl)
        {
            std::cout << "PARSED " << typeName.str() << std::endl;
            DeclGroupRef DG = ADecl.get();
            for (auto i = DG.begin(), e = DG.end(); i != e; ++i)
            {
                Decl *D = *i;    
                D->dumpColor();
            }
        }
        else
        {
            std::cout << "FAILED TO PARSE: " << typeName.str() << std::endl;
        }
    }
    return qt;
}