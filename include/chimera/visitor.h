#ifndef __CHIMERA_VISITOR_H__
#define __CHIMERA_VISITOR_H__

#include "chimera/configuration.h"

#include <set>
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

    bool shouldVisitImplicitCode() const;
    bool shouldVisitTemplateInstantiations() const;
    bool VisitDecl(clang::Decl *decl);

protected:
    bool GenerateCXXRecord(clang::CXXRecordDecl *decl);
    bool GenerateEnum(clang::EnumDecl *decl);
    bool GenerateGlobalVar(clang::VarDecl *decl);
    bool GenerateGlobalFunction(clang::FunctionDecl *decl);

private:
    clang::PrintingPolicy printing_policy_;
    std::unique_ptr<CompiledConfiguration> config_;

    std::set<const clang::CXXRecordDecl *> traversed_class_decls_;
};

} // namespace chimera

#endif // __CHIMERA_VISITOR_H__
