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

    virtual bool TraverseNamespaceDecl(clang::NamespaceDecl *ns);
    virtual bool VisitFunctionDecl(clang::FunctionDecl *func);
    virtual bool VisitStmt(clang::Stmt *st);
    virtual bool VisitReturnStmt(clang::ReturnStmt *ret);
    virtual bool VisitCallExpr(clang::CallExpr *call);

private:
    clang::ASTContext *context_;
    std::unique_ptr<CompiledConfiguration> config_;
};

} // namespace chimera

#endif // __CHIMERA_VISITOR_H__
