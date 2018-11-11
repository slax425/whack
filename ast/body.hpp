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
#ifndef WHACK_BODY_HPP
#define WHACK_BODY_HPP

#pragma once

#include "ast.hpp"
#include "deferstmt.hpp"
#include "ident.hpp"
#include "tags.hpp"
#include <llvm/IR/CFG.h>

namespace whack::ast {

class Body final : public Stmt {
public:
  explicit Body(const mpc_ast_t* const ast) : Stmt(kBody), state_{ast->state} {
    auto idx = 1;
    if (getInnermostAstTag(ast->children[0]) == "tags") {
      tags_ = std::make_unique<Tags>(ast->children[0]);
      ++idx;
    }
    for (; idx < ast->children_num - 1; ++idx) {
      statements_.emplace_back(getStmt(ast->children[idx]));
    }
  }

  llvm::Error codegen(llvm::IRBuilder<>& builder) const final {
    begin_ = builder.GetInsertBlock();
    for (const auto& stmt : statements_) {
      if (auto err = stmt->codegen(builder)) {
        return err;
      }
      if (llvm::isa<Defer>(stmt.get())) {
        deferrals_.insert(deferrals_.begin(),
                          std::pair{builder.GetInsertBlock(), stmt.get()});
      }
    }
    end_ = builder.GetInsertBlock();
    return llvm::Error::success();
  }

  llvm::Error runScopeExit(llvm::IRBuilder<>& builder) const final {
    llvm::IRBuilder<>::InsertPointGuard{builder};
    const auto current = builder.GetInsertBlock();
    for (const auto& stmt : statements_) {
      if (!llvm::isa<Defer>(stmt.get())) {
        if (auto err = stmt->runScopeExit(builder)) {
          return err;
        }
      }
    }

    for (const auto& [block, defer] : deferrals_) {
      if (auto err = applyDefer(builder, current, block, defer)) {
        return err;
      }
    }

    if (tags_) {
      return this->handleTags(builder);
    }
    return llvm::Error::success();
  }

  inline static bool classof(const Stmt* const stmt) {
    return stmt->getKind() == kBody;
  }

private:
  const mpc_state_t state_;
  std::unique_ptr<Tags> tags_;
  small_vector<std::unique_ptr<Stmt>> statements_;
  mutable llvm::BasicBlock* begin_;
  mutable llvm::BasicBlock* end_;
  using deferral_info_t = std::pair<llvm::BasicBlock*, Stmt*>;
  mutable std::vector<deferral_info_t> deferrals_;

  static llvm::Error applyDefer(llvm::IRBuilder<>& builder,
                                llvm::BasicBlock* const current,
                                llvm::BasicBlock* const block,
                                const Stmt* const defer) {
    if (block == current) {
      builder.SetInsertPoint(block);
      return defer->runScopeExit(builder);
    }
    for (const auto successor : llvm::successors(block)) {
      if (successor == current || llvm::succ_empty(successor)) {
        builder.SetInsertPoint(successor);
        if (auto err = defer->runScopeExit(builder)) {
          return err;
        }
      } else {
        if (auto err = applyDefer(builder, current, successor, defer)) {
          return err;
        }
      }
    }
    return llvm::Error::success();
  }

  llvm::Error handleTags(llvm::IRBuilder<>& builder) const {
    static llvm::StringMap<llvm::Attribute::AttrKind> InternalTags{
        {"noinline", llvm::Attribute::AttrKind::NoInline},
        {"inline", llvm::Attribute::AttrKind::InlineHint},
        {"mustinline", llvm::Attribute::AttrKind::AlwaysInline},
        {"noreturn", llvm::Attribute::AttrKind::NoReturn}};

    const auto func = builder.GetInsertBlock()->getParent();
    for (const auto& [name, args] : tags_->get()) {
      if (name.index() == 0) { // <scoperes>
        llvm_unreachable("not implemented!");
      } else { // <ident>
        const auto& tag = std::get<Ident>(name).name();
        if (!InternalTags.count(tag)) { // @todo: Other tag kinds
          return error("tag `{}` not implemented at line {}", tag.str(),
                       state_.row + 1);
        }
        func->addFnAttr(InternalTags[tag]);
      }
    }
    return llvm::Error::success();
  }
};

} // end namespace whack::ast

#endif // WHACK_BODY_HPP
