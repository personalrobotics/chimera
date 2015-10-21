#include "chimera/util.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/Parser.h>
#include <clang/Sema/Sema.h>
#include <iostream>
#include <sstream>

using namespace clang;

namespace
{
/**
 * Empty QualType used when returning a type-resolution failure.
 */
static const clang::QualType emptyType_;

/**
 * Generates incremental typedef names to avoid namespace conflicts.
 *
 * This is used by functions that need to compile a temporary typedef to
 * resolve a type within the AST.
 */
std::string generateUniqueTypedefName()
{
    // Use a static variable to generate non-duplicate names.
    static size_t counter = 0;

    std::stringstream ss;
    ss << "chimera_typedef_" << (counter++);
    return ss.str();
}

}

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

const QualType
chimera::util::resolveType(clang::CompilerInstance *ci,
                           const llvm::StringRef typeStr)
{
    auto decl = chimera::util::resolveDeclaration(
        ci, "typedef " + typeStr.str() + " " + generateUniqueTypedefName());
    if (!decl)
        return emptyType_;

    if (!isa<TypedefDecl>(decl))
    {
        std::cerr << "Expected 'typedef' declaration, found '"
                  << decl->getNameAsString() << "'." << std::endl;
        return emptyType_;
    }

    auto typedef_decl = cast<TypedefDecl>(decl);
    return typedef_decl->getUnderlyingType().getCanonicalType();
}

const RecordDecl*
chimera::util::resolveRecord(clang::CompilerInstance *ci,
                             const llvm::StringRef recordStr)
{
    auto type = chimera::util::resolveType(ci, recordStr).getTypePtrOrNull();
    if (!type)
        return nullptr;

    return cast<RecordDecl>(type->getAsCXXRecordDecl()->getCanonicalDecl());
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
