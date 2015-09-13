#include "chimera/visitor.h"

#include <algorithm>
#include <iostream>
#include <string>

using namespace chimera;
using namespace clang;

chimera::Visitor::Visitor(CompilerInstance *CI)
: astContext(&(CI->getASTContext()))
{
    // Do nothing.
}

bool chimera::Visitor::TraverseNamespaceDecl(NamespaceDecl *ns)
{
    return clang::RecursiveASTVisitor<Visitor>::TraverseNamespaceDecl(ns);
}

bool chimera::Visitor::VisitFunctionDecl(FunctionDecl *func)
{
    const auto funcName = func->getNameInfo().getName().getAsString();
    //std::cout << "** Rewrote function def: " << funcName << std::endl;
    return true;
}

bool chimera::Visitor::VisitStmt(Stmt *st)
{
    if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st))
    {
        //std::cout << "** Rewrote ReturnStmt" << std::endl;
    }

    if (CallExpr *call = dyn_cast<CallExpr>(st))
    {
        //std::cout << "** Rewrote function call" << std::endl;
    }
    return true;
}

bool chimera::Visitor::VisitReturnStmt(ReturnStmt *ret) 
{
    //std::cout << "** Rewrote ReturnStmt" << std::endl;
    return true;
}

bool chimera::Visitor::VisitCallExpr(CallExpr *call) {
    //std::cout << "** Rewrote function call" << std::endl;
    return true;
}

