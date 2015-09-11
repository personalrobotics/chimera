#ifndef __CHIMERA_VISITOR_H__
#define __CHIMERA_VISITOR_H__

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
public:
    explicit Visitor(clang::CompilerInstance *CI);

    virtual bool VisitFunctionDecl(clang::FunctionDecl *func);
    virtual bool VisitStmt(clang::Stmt *st);
    virtual bool VisitReturnStmt(clang::ReturnStmt *ret);
    virtual bool VisitCallExpr(clang::CallExpr *call);

private:
    clang::ASTContext *astContext; // used for getting additional AST info
};

} // namespace chimera

#endif // __CHIMERA_VISITOR_H__