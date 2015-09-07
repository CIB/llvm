//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
using namespace llvm;

#define DEBUG_TYPE "custom_inliner"

namespace {
  struct CustomInliner : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    CustomInliner() : ModulePass(ID) {}

    bool runOnModule(Module &M) override {
      for(auto &F : M.functions()) {
        if(F.getName() == "openDatabase") {
            recursiveInlineFunctions(F);
        }
      }
      return false;
    }
    
    bool inlineNextCallSite(Function &F) {
        for(auto& BB : F) {
            for(auto& I : BB) {
                CallSite CS(cast<Value>(&I));
                if (!CS || isa<IntrinsicInst>(I))
                  continue;
                if (auto Callee = CS.getCalledFunction())
                  if (Callee->isDeclaration())
                    continue;
                
                InlineFunctionInfo InlineInfo;
                
                // Try to inline the function.
                llvm::outs() << "Attempting to inline " << I << "\n";
                if(InlineFunction(CS, InlineInfo, false)) {
                    llvm::outs() << "Success!\n";
                    return true;
                }
                llvm::outs() << "Failure!\n";
            }
        }
        
        return false;
    }
    
    void recursiveInlineFunctions(Function &F) {
        bool do_again = true;
        while(do_again) {
            do_again = inlineNextCallSite(F);
        }
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
    }
  };
}

char CustomInliner::ID = 0;
static RegisterPass<CustomInliner>
Y("custom-inliner", "Custom inliner that tries to inline as much as possible into a specified function.");
