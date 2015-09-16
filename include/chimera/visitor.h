#ifndef __CHIMERA_VISITOR_H__
#define __CHIMERA_VISITOR_H__

#include "chimera/configuration.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
public:
    Visitor(clang::CompilerInstance *ci,
            std::unique_ptr<CompiledConfiguration> cc);

    virtual bool VisitDecl(clang::Decl *decl);

protected:
    void GenerateCXXRecord(clang::CXXRecordDecl *decl);
    void GenerateCXXMethod(
        clang::CXXRecordDecl *class_decl, clang::CXXMethodDecl *decl);

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
