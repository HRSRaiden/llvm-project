//===--- ExecuteCompilerInvocation.cpp ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file holds ExecuteCompilerInvocation(). It is split into its own file to
// minimize the impact of pulling in essentially everything else in Flang.
//
//===----------------------------------------------------------------------===//

#include "flang/Frontend/CompilerInstance.h"
#include "flang/Frontend/FrontendActions.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/BuryPointer.h"
#include "llvm/Support/CommandLine.h"

namespace Fortran::frontend {

static std::unique_ptr<FrontendAction> CreateFrontendBaseAction(
    CompilerInstance &ci) {

  ActionKind ak = ci.frontendOpts().programAction_;
  switch (ak) {
  case InputOutputTest:
    return std::make_unique<InputOutputTestAction>();
    break;
  case PrintPreprocessedInput:
    return std::make_unique<PrintPreprocessedAction>();
    break;
  case ParseSyntaxOnly:
    return std::make_unique<ParseSyntaxOnlyAction>();
  case EmitObj:
    return std::make_unique<EmitObjAction>();
    break;
  case DebugUnparse:
    return std::make_unique<DebugUnparseAction>();
    break;
  case DebugUnparseNoSema:
    return std::make_unique<DebugUnparseNoSemaAction>();
    break;
  case DebugUnparseWithSymbols:
    return std::make_unique<DebugUnparseWithSymbolsAction>();
    break;
  case DebugDumpSymbols:
    return std::make_unique<DebugDumpSymbolsAction>();
    break;
  case DebugDumpParseTree:
    return std::make_unique<DebugDumpParseTreeAction>();
    break;
  case DebugDumpParseTreeNoSema:
    return std::make_unique<DebugDumpParseTreeNoSemaAction>();
    break;
  case DebugDumpProvenance:
    return std::make_unique<DebugDumpProvenanceAction>();
    break;
  case DebugDumpParsingLog:
    return std::make_unique<DebugDumpParsingLogAction>();
    break;
  case DebugMeasureParseTree:
    return std::make_unique<DebugMeasureParseTreeAction>();
    break;
  case DebugPreFIRTree:
    return std::make_unique<DebugPreFIRTreeAction>();
    break;
  case GetSymbolsSources:
    return std::make_unique<GetSymbolsSourcesAction>();
    break;
  default:
    break;
    // TODO:
    // case RunPreprocessor:
    // case ParserSyntaxOnly:
    // case EmitLLVM:
    // case EmitLLVMOnly:
    // case EmitCodeGenOnly:
    // (...)
  }
  return 0;
}

std::unique_ptr<FrontendAction> CreateFrontendAction(CompilerInstance &ci) {
  // Create the underlying action.
  std::unique_ptr<FrontendAction> act = CreateFrontendBaseAction(ci);
  if (!act)
    return nullptr;

  return act;
}
bool ExecuteCompilerInvocation(CompilerInstance *flang) {
  // Honor -help.
  if (flang->frontendOpts().showHelp_) {
    clang::driver::getDriverOptTable().PrintHelp(llvm::outs(),
        "flang-new -fc1 [options] file...", "LLVM 'Flang' Compiler",
        /*Include=*/clang::driver::options::FC1Option,
        /*Exclude=*/llvm::opt::DriverFlag::HelpHidden,
        /*ShowAllAliases=*/false);
    return true;
  }

  // Honor -version.
  if (flang->frontendOpts().showVersion_) {
    llvm::cl::PrintVersionMessage();
    return true;
  }

  // Create and execute the frontend action.
  std::unique_ptr<FrontendAction> act(CreateFrontendAction(*flang));
  if (!act)
    return false;

  bool success = flang->ExecuteAction(*act);
  return success;
}

} // namespace Fortran::frontend
