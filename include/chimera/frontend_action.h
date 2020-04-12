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

// TODO: Check the specific version at which this API change occurs.
//
// At some point between Clang 6 and Clang 10, the return value for
// FrontendActionFactory::create() changed from a raw pointer to a
// std::unique_ptr, but we haven't specifically checked which version
// the change occurred in yet.
#if LLVM_VERSION_AT_LEAST(9, 0, 0)
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
