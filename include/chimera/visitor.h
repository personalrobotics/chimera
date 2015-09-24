#ifndef __CHIMERA_VISITOR_H__
#define __CHIMERA_VISITOR_H__

#include "chimera/configuration.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

// Forward declare LLVM raw_ostream, as per:
// http://llvm.org/docs/CodingStandards.html#use-raw-ostream
namespace llvm
{
class raw_pwrite_stream;
}

namespace chimera
{

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
public:
    Visitor(clang::CompilerInstance *ci,
            std::unique_ptr<CompiledConfiguration> cc);

    virtual bool VisitDecl(clang::Decl *decl);

protected:
    bool GenerateCXXRecord(clang::CXXRecordDecl *decl);
    bool GenerateCXXConstructor(llvm::raw_pwrite_stream &stream,
                                clang::CXXRecordDecl *class_decl,
                                clang::CXXConstructorDecl *decl);
    bool GenerateCXXMethod(llvm::raw_pwrite_stream &stream,
                           clang::CXXRecordDecl *class_decl,
                           clang::CXXMethodDecl *decl);
    bool GenerateField(llvm::raw_pwrite_stream &stream,
                       clang::CXXRecordDecl *class_decl,
                       clang::FieldDecl *decl);

    bool GenerateEnum(clang::EnumDecl *decl);
    bool GenerateGlobalVar(clang::VarDecl *decl);

private:
    bool IsEnclosed(clang::Decl *decl) const;
    std::vector<std::string> GetBaseClassNames(clang::CXXRecordDecl *decl) const;
    std::vector<std::pair<std::string, std::string>> GetParameterNames(
        clang::CXXMethodDecl *decl) const;

    clang::ASTContext *context_;
    std::unique_ptr<CompiledConfiguration> config_;
};

} // namespace chimera

#endif // __CHIMERA_VISITOR_H__
