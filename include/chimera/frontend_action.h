#ifndef __CHIMERA_FRONTEND_ACTION_H__
#define __CHIMERA_FRONTEND_ACTION_H__

#include "chimera/configuration.h"
#include "chimera/util.h"

#include <memory>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

namespace chimera
{

/**
 * Front-end that runs the Chimera AST consumer on the provided source.
 */
class FrontendAction : public clang::ASTFrontendAction
{
public:
    // Overrides the constructor in order to receive ChimeraConfiguration.
    FrontendAction(const chimera::Configuration &config);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI, clang::StringRef file) override;

protected:
    const chimera::Configuration &config_;
};

// Create a custom action factory that forwards the ChimeraConfiguration.
class ChimeraFrontendActionFactory
  : public clang::tooling::FrontendActionFactory
{
public:
    ChimeraFrontendActionFactory(const chimera::Configuration &config);

// Between Clang 9 and Clang 10, the return value for
// FrontendActionFactory::create() changed from raw pointer to std::unique_ptr.
#if LLVM_VERSION_AT_LEAST(10, 0, 0)
    std::unique_ptr<clang::FrontendAction> create() override;
#else
    clang::FrontendAction *create() override;
#endif

protected:
    const chimera::Configuration &config_;
};

/**
 * Custom frontend factory that forwards a ChimeraConfiguration.
 */
std::unique_ptr<clang::tooling::FrontendActionFactory> newFrontendActionFactory(
    const chimera::Configuration &config);

} // namespace chimera

#endif // __CHIMERA_FRONTEND_ACTION_H__
