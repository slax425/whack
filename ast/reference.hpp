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
#ifndef WHACK_REFERENCE_HPP
#define WHACK_REFERENCE_HPP

#pragma once

#include "ast.hpp"

namespace whack::ast {

class Reference final : public Factor {
public:
  explicit Reference(const mpc_ast_t* const ast)
      : Factor(kReference), state_{ast->state}, variable_{getFactor(
                                                    ast->children[0])} {}

  llvm::Expected<llvm::Value*> codegen(llvm::IRBuilder<>&) const final {
    return error("References not implemented at line {}", state_.row + 1);
  }

private:
  const mpc_state_t state_;
  std::unique_ptr<Factor> variable_;
};

} // end namespace whack::ast

#endif // WHACK_REFERENCE_HPP
