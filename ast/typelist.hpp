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
#ifndef WHACK_TYPELIST_HPP
#define WHACK_TYPELIST_HPP

#pragma once

#include "ast.hpp"

namespace whack::ast {

class TypeList final : public AST {
public:
  explicit constexpr TypeList(const mpc_ast_t* const ast) noexcept
      : ast_{ast} {}

  llvm::Expected<typelist_t> codegen(const llvm::Module* const module) const {
    bool variadic = false;
    small_vector<llvm::Type*> types;
    const auto tag = getInnermostAstTag(ast_);
    if (tag == "variadictype") {
      auto type = getType(ast_->children[0], module);
      if (!type) {
        return type.takeError();
      }
      types.push_back(*type);
      variadic = true;
    } else if (tag != "typelist" || !ast_->children_num) {
      auto type = getType(ast_, module);
      if (!type) {
        return type.takeError();
      }
      types = {*type};
    } else {
      for (auto i = 0; i < ast_->children_num; i += 2) {
        const auto ref = ast_->children[i];
        if (getInnermostAstTag(ref) == "variadictype") {
          auto type = getType(ref->children[0], module);
          if (!type) {
            return type.takeError();
          }
          types.push_back(*type);
          variadic = true;
        } else {
          auto type = getType(ref, module);
          if (!type) {
            return type.takeError();
          }
          types.push_back(*type);
        }
      }
    }
    return typelist_t{std::move(types), variadic};
  }

private:
  const mpc_ast_t* const ast_;
};

inline static llvm::Expected<typelist_t>
getTypeList(const mpc_ast_t* const ast, const llvm::Module* const module) {
  return TypeList{ast}.codegen(module);
}

} // end namespace whack::ast

#endif // WHACK_TYPELIST_HPP
