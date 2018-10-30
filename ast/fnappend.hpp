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
#ifndef WHACK_FNAPPEND_HPP
#define WHACK_FNAPPEND_HPP

#pragma once

#include "ast.hpp"

namespace whack::ast {

class FnAppend final : public Factor {
public:
  explicit FnAppend(const mpc_ast_t* const ast) : Factor(kFnAppend) {}

  llvm::Expected<llvm::Value*> codegen(llvm::IRBuilder<>& builder) const final {
    llvm_unreachable("not implemented");
  }

  inline static bool classof(const Factor* const factor) {
    return factor->getKind() == kFnAppend;
  }
};

#endif // WHACK_FNAPPEND_HPP
