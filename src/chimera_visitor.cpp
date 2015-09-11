#include "chimera/chimera_visitor.h"
#include <iostream>
#include <string>

using namespace chimera;
using namespace clang;

ChimeraVisitor::ChimeraVisitor(CompilerInstance *CI)
: astContext(&(CI->getASTContext()))
{
    // Do nothing.
}

bool ChimeraVisitor::VisitFunctionDecl(FunctionDecl *func)
{
    std::string funcName = func->getNameInfo().getName().getAsString();
    std::cout << "** Rewrote function def: " << funcName << std::endl;
    return true;
}

bool ChimeraVisitor::VisitStmt(Stmt *st)
{
    if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st))
    {
        std::cout << "** Rewrote ReturnStmt" << std::endl;
    }

    if (CallExpr *call = dyn_cast<CallExpr>(st))
    {
        std::cout << "** Rewrote function call" << std::endl;
    }
    return true;
}

bool ChimeraVisitor::VisitReturnStmt(ReturnStmt *ret) 
{
    std::cout << "** Rewrote ReturnStmt" << std::endl;
    return true;
}

bool ChimeraVisitor::VisitCallExpr(CallExpr *call) {
    std::cout << "** Rewrote function call" << std::endl;
    return true;
}

