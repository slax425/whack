/**
 * Copyright 2018 Onchere Bironga
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef WHACK_IDENT_HPP
#define WHACK_IDENT_HPP

#pragma once

#include "ast.hpp"
#include "integral.hpp"
#include "metadata.hpp"
#include "structmember.hpp"
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/Support/Error.h>

namespace whack::ast {

class Ident final : public Factor {
public:
  explicit Ident(const mpc_ast_t* const ast)
      : Factor(kIdent), state_{ast->state}, name_{ast->contents} {}

  llvm::Expected<llvm::Value*> codegen(llvm::IRBuilder<>& builder) const final {
    const auto func = builder.GetInsertBlock()->getParent();
    // variable
    if (auto var = func->getValueSymbolTable()->lookup(name_)) {
      return var;
    }

    // captured variables in closure
    const auto module = func->getParent();
    if (func->getName().startswith("::closure")) {
      const auto env = func->getValueSymbolTable()->lookup(".env");
      const auto structure = env->getType()->getPointerElementType();
      const auto idx =
          StructMember::getIndex(*module, structure->getStructName(), name_);
      if (idx) {
        const auto ptr =
            builder.CreateStructGEP(structure, env, idx.value(), name_);
        return builder.CreateLoad(ptr);
      }
    }

    // free function
    if (auto func = module->getFunction(name_)) {
      return func;
    }

    if (name_ == "_") {
      llvm::GlobalVariable* discard;
      if (discard = module->getGlobalVariable("_"); !discard) {
        static constexpr auto linkage = llvm::Function::InternalLinkage;
        (void)llvm::GlobalVariable{BasicTypes["char"], true, linkage, nullptr,
                                   "_"};
        discard = module->getGlobalVariable("_");
      }
      return llvm::cast<llvm::Value>(discard);
    }

    return error("variable `{}` does not exist in "
                 "scope at line {}",
                 name_.data(), state_.row + 1);
  }

  inline const auto& name() const { return name_; }

  static llvm::Error isUnique(const llvm::Module* const module,
                              llvm::StringRef name, const mpc_state_t state) {
    const auto line = state.row + 1;
    if (module->getFunction(name)) {
      return error("identifier `{}` already exists as a function "
                   "at line {}",
                   name.data(), line);
    }

    if (getMetadataOperand(*module, "structures", name)) {
      return error("identifier `{}` already exists as a structure "
                   "at line {}",
                   name.data(), line);
    }

    if (getMetadataOperand(*module, "interfaces", name)) {
      return error("identifier `{}` already exists as an interface "
                   "at line {}",
                   name.data(), line);
    }

    if (getMetadataOperand(*module, "classes", name)) {
      return error("identifier `{}` already exists as a data class "
                   "at line {}",
                   name.data(), line);
    }

    if (getMetadataOperand(*module, "aliases", name)) {
      return error("identifier `{}` already exists as an alias "
                   "at line {}",
                   name.data(), line);
    }
    return llvm::Error::success();
  }

  static llvm::Error isUnique(const llvm::IRBuilder<>& builder,
                              llvm::StringRef name, const mpc_state_t state) {
    const auto line = state.row + 1;
    if (name == "_") {
      return error("identifier `{}` is reserved for discarded "
                   "assignment values at line {}",
                   line);
    }

    if (isReserved(name)) {
      return error("identifier `{}` is reserved at line {}", name.data(), line);
    }

    const auto func = builder.GetInsertBlock()->getParent();
    if (func->getValueSymbolTable()->lookup(name)) {
      return error("identifier `{}` already exists in function `{}` "
                   "at line {}",
                   name.data(), func->getName().str(), line);
    }

    return isUnique(func->getParent(), name, state);
  }

  inline static bool classof(const Factor* const factor) {
    return factor->getKind() == kIdent;
  }

private:
  const mpc_state_t state_;
  const llvm::StringRef name_;
};

} // end namespace whack::ast

#endif // WHACK_IDENT_HPP
