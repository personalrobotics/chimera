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
    bool GenerateCXXRecord(clang::CXXRecordDecl *decl);
    bool GenerateCXXConstructor(chimera::Stream &stream,
                                clang::CXXRecordDecl *class_decl,
                                clang::CXXConstructorDecl *decl);
    bool GenerateFunction(chimera::Stream &stream,
                           clang::CXXRecordDecl *class_decl,
                           clang::FunctionDecl *decl);
    bool GenerateField(chimera::Stream &stream,
                       clang::CXXRecordDecl *class_decl,
                       clang::FieldDecl *decl);
    bool GenerateStaticField(chimera::Stream &stream,
                             clang::CXXRecordDecl *class_decl,
                             clang::VarDecl *decl);

    bool GenerateClassTemplate(clang::ClassTemplateDecl *decl);
    bool GenerateEnum(clang::EnumDecl *decl);
    bool GenerateGlobalVar(clang::VarDecl *decl);
    bool GenerateGlobalFunction(clang::FunctionDecl *decl);

private:
    bool IsEnclosed(clang::Decl *decl) const;
    std::vector<std::string> GetBaseClassNames(clang::CXXRecordDecl *decl) const;
    std::vector<std::pair<std::string, std::string>> GetParameterNames(
        clang::FunctionDecl *decl) const;

    clang::ASTContext *context_;
    clang::PrintingPolicy printing_policy_;
    std::unique_ptr<CompiledConfiguration> config_;
};

} // namespace chimera

#endif // __CHIMERA_VISITOR_H__
