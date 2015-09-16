#include "chimera/visitor.h"

#include <algorithm>
#include <iostream>
#include <string>

using namespace chimera;
using namespace clang;

chimera::Visitor::Visitor(clang::CompilerInstance *ci,
                          std::unique_ptr<CompiledConfiguration> cc)
: context_(&(ci->getASTContext()))
, config_(std::move(cc))
{
    // Do nothing.
}

bool chimera::Visitor::TraverseNamespaceDecl(NamespaceDecl *ns)
{
    // Filter over the namespaces and only traverse ones that are enclosed
    // by one of the configuration namespaces.
    auto namespaces = config_->GetNamespaces();
    for(auto it = namespaces.begin(); it != namespaces.end(); ++it)
    {
        if ((*it)->Encloses(ns))
        {
            clang::RecursiveASTVisitor<Visitor>::TraverseNamespaceDecl(ns);
        }
    }

    return true;
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
