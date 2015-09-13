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
        return nullptr;

    // Create a new parser to handle this type parsing.
    Preprocessor &preprocessor = ci->getPreprocessor();
    Sema &sema = ci->getSema();
    Parser parser(preprocessor, sema, /* SkipFunctionBodies = */ false);

    // Set up the preprocessor to only care about incrementally handling type.
    preprocessor.getDiagnostics().setIgnoreAllWarnings(true);
    preprocessor.getDiagnostics().setSeverityForGroup(diag::Flavor::WarningOrError,
                                                      "out-of-line-declaration",
                                                      diag::Severity::Ignored);
    preprocessor.enableIncrementalProcessing();
    const_cast<LangOptions&>(preprocessor.getLangOpts()).SpellChecking = 0;

    // Put the type string into a buffer and run it through the preprocessor.
    FileID fid = sema.getSourceManager().createFileID(
        llvm::MemoryBuffer::getMemBufferCopy(declStr.str() + ";",
                                             "chimera.util.resolveDeclaration"));
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

                return (isa<NamedDecl>(D)) ? cast<NamedDecl>(D) : nullptr;
            }
        }
    }

    std::cerr << "FAILED TO PARSE: " << declStr.str() << std::endl;
    return nullptr;
}

const RecordDecl*
chimera::util::resolveRecord(clang::CompilerInstance *ci,
                             const llvm::StringRef recordStr)
{
    auto decl = chimera::util::resolveDeclaration(ci, "using " + recordStr.str() + " ;");
    if (!decl)
        return nullptr;

    if (!isa<UsingDecl>(decl))
    {
        std::cerr << "Expected 'using' declaration, found '"
                  << decl->getNameAsString() << "'." << std::endl;
        return nullptr;
    }

    auto using_decl = cast<UsingDecl>(decl);
    if (using_decl->shadow_size() != 1)
    {
        std::cerr << "Unexpected shadow declarations when resolving '"
                  << recordStr.str() << "' expected 1, found "
                  << using_decl->shadow_size() << "." << std::endl;
        return nullptr;
    }

    auto shadow_decl = cast<UsingShadowDecl>(*(using_decl->shadow_begin()));
    if (!isa<RecordDecl>(shadow_decl->getTargetDecl()))
    {
        std::cerr << "Expected 'using class' declaration, found '"
                  << shadow_decl->getTargetDecl()->getNameAsString()
                  << "'." << std::endl;
        return nullptr;
    }

    return cast<RecordDecl>(shadow_decl->getTargetDecl()->getCanonicalDecl());
}

const NamespaceDecl*
chimera::util::resolveNamespace(clang::CompilerInstance *ci,
                                const llvm::StringRef nsStr)
{
    auto decl = chimera::util::resolveDeclaration(ci, "using namespace " + nsStr.str());
    if (!decl)
        return nullptr;

    if (!isa<UsingDirectiveDecl>(decl))
    {
        std::cerr << "Expected 'using namespace' declaration, found '"
                  << decl->getNameAsString() << "'." << std::endl;
        return nullptr;
    }

    return cast<NamespaceDecl>(
        cast<UsingDirectiveDecl>(decl)
            ->getNominatedNamespace()->getCanonicalDecl());
}
