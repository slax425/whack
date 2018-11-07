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
#ifndef WHACK_INTERFACE_HPP
#define WHACK_INTERFACE_HPP

#pragma once

#include "ast.hpp"
#include "identifier.hpp"
#include "structmember.hpp"

namespace whack::ast {

class Interface final : public AST {
public:
  explicit Interface(const mpc_ast_t* const ast)
      : state_{ast->state}, name_{ast->children[1]->contents} {
    const auto ref = ast->children[2];
    auto idx = 1;
    if (std::string_view(ref->children[idx]->contents) == ":") {
      for (idx = 2; idx < ref->children_num; ++idx) {
        const auto inherit = ref->children[idx];
        const std::string_view view{inherit->contents};
        if (view == ",") {
          continue;
        }
        if (view == "{") {
          break;
        }
        inherits_.emplace_back(getIdentifier(inherit));
      }
    }
    for (++idx; idx < ref->children_num - 1; idx += 2) {
      Type type{ref->children[idx]};
      if (getInnermostAstTag(ref->children[idx + 1]) == "ident") {
        std::string name{ref->children[idx + 1]->contents};
        functions_.emplace_back(
            std::pair{std::move(type), std::optional{std::move(name)}});
        ++idx;
      } else {
        functions_.emplace_back(std::pair{std::move(type), std::nullopt});
      }
    }
  }

  llvm::Error codegen(llvm::Module* const module) const {
    if (auto err = Ident::isUnique(module, name_, state_)) {
      return err;
    }
    small_vector<llvm::Type*> funcs;
    small_vector<std::string> funcNames;
    small_vector<std::pair<llvm::MDNode*, uint64_t>> metadata;
    auto& ctx = module->getContext();
    llvm::MDBuilder MDBuilder{ctx};
    for (const auto& inherit : inherits_) {
      switch (inherit.index()) {
      case 1:
        // const auto& name = std::get<ScopeRes>(inherit);
        return error("cross-module interface inheritance not "
                     "implemented at line {}",
                     state_.row + 1);
      case 2: {
        const auto& name = std::get<Ident>(inherit).name();
        auto funcsInfo = getFuncsInfo(module, name, state_);
        if (!funcsInfo) {
          return funcsInfo.takeError();
        }
        std::tie(funcs, funcNames) = std::move(*funcsInfo);
        for (size_t i = 0; i < funcs.size(); ++i) {
          const auto domain = reinterpret_cast<llvm::MDNode*>(
              MDBuilder.createConstant(llvm::Constant::getNullValue(funcs[i])));
          const auto nameMD =
              MDBuilder.createAnonymousAliasScope(domain, funcNames[i]);
          metadata.emplace_back(std::pair{nameMD, i});
        }
      }
      }
    }

    for (const auto& [type, name] : functions_) {
      const auto fnType = type.codegen(module)->getPointerTo(0);
      const auto domain = reinterpret_cast<llvm::MDNode*>(
          MDBuilder.createConstant(llvm::Constant::getNullValue(fnType)));
      const auto nm = name ? name.value() : std::to_string(funcs.size());
      if (std::find(funcNames.begin(), funcNames.end(), nm) !=
          funcNames.end()) {
        return error("interface `{}` already declares function `{}` "
                     "at line {}",
                     name_, nm, state_.row + 1);
      }
      funcNames.push_back(nm);
      const auto nameMD = MDBuilder.createAnonymousAliasScope(domain, nm);
      metadata.emplace_back(std::pair{nameMD, funcs.size()});
      funcs.push_back(fnType);
    }

    const auto impl =
        llvm::StructType::create(ctx, funcs, "interface::" + name_);
    const auto interfaceMD =
        MDBuilder.createTBAAStructTypeNode(name_, metadata);
    module->getOrInsertNamedMetadata("interfaces")->addOperand(interfaceMD);
    Structure::addMetadata(module, impl->getName(), funcNames);
    return llvm::Error::success();
  }

  inline const auto& name() const { return name_; }
  inline const auto& functions() const { return functions_; }

  using funcs_info_t =
      std::pair<small_vector<llvm::Type*>, small_vector<std::string>>;

  inline static llvm::Expected<funcs_info_t>
  getFuncsInfo(const llvm::Module* const module, llvm::Type* const interface,
               const mpc_state_t state) {
    return getFuncsInfo(module, getName(interface), state);
  }

  static llvm::Expected<funcs_info_t>
  getFuncsInfo(const llvm::Module* const module, llvm::StringRef interfaceName,
               const mpc_state_t state) {
    small_vector<llvm::Type*> funcs;
    small_vector<std::string> funcNames;
    if (auto MD = getMetadataOperand(*module, "interfaces", interfaceName)) {
      for (unsigned i = 1; i < MD.value()->getNumOperands(); i += 2) {
        const auto interfaceMD =
            reinterpret_cast<llvm::MDNode*>(MD.value()->getOperand(i).get());
        const auto func = interfaceMD->getOperand(1).get();
        funcs.push_back(
            llvm::mdconst::extract<llvm::Constant>(func)->getType());
        const auto funcName =
            reinterpret_cast<llvm::MDString*>(interfaceMD->getOperand(2).get())
                ->getString();
        funcNames.emplace_back(funcName.str());
      }
    } else {
      return error("interface `{}` does not exist at line {}",
                   interfaceName.str(), state.row + 1);
    }
    return std::pair{std::move(funcs), std::move(funcNames)};
  }

  static llvm::Expected<small_vector<llvm::Function*>>
  implements(llvm::IRBuilder<>& builder, llvm::Type* const interface,
             llvm::Value* const value, const mpc_state_t state) {
    const auto [type, isStruct] = Type::isStructKind(value->getType());
    if (!isStruct) {
      return error("expected value `{}` to be a struct kind at line {}",
                   value->getName().str(), state.row + 1);
    }
    const auto structName = type->getStructName().str();
    const auto module = builder.GetInsertBlock()->getModule();
    auto funcsInfo = getFuncsInfo(module, interface, state);
    if (!funcsInfo) {
      return funcsInfo.takeError();
    }
    const auto& [funcs, funcNames] = *funcsInfo;
    small_vector<llvm::Function*> funcsImpl;
    for (size_t i = 0; i < funcs.size(); ++i) {
      const auto funcType = funcs[i];
      const auto& funcName = funcNames[i];
      if (auto structFunc = module->getFunction(
              format("struct::{}::{}", structName, funcName))) {
        const auto func = StructMember::bindThis(builder, structFunc, value);
        if (func->getType() != funcType) {
          return error("struct `{}` does not implement interface `{}` "
                       "(type mismatch for function `{}`) at line {}",
                       structName, getName(interface).str(), funcName,
                       state.row + 1);
        }
        funcsImpl.push_back(func);
      } else {
        return error("struct `{}` does not implement interface `{}` "
                     "(no implementation found for function `{}`) at line {}",
                     structName, getName(interface).str(), funcName,
                     state.row + 1);
      }
    }
    return std::move(funcsImpl);
  }

  static llvm::Expected<llvm::Value*> cast(llvm::IRBuilder<>& builder,
                                           llvm::Type* const interface,
                                           llvm::Value* const value,
                                           const mpc_state_t state) {
    const auto [interfaceType, isStruct] = Type::isStructKind(interface);
    assert(isStruct);
    auto impl = implements(builder, interfaceType, value, state);
    if (!impl) {
      return impl.takeError();
    }
    const auto& funcsImpl = *impl;
    auto interfaceImpl = builder.CreateAlloca(interfaceType, 0, nullptr, "");
    for (size_t i = 0; i < funcsImpl.size(); ++i) {
      const auto ptr =
          builder.CreateStructGEP(interfaceType, interfaceImpl, i, "");
      builder.CreateStore(funcsImpl[i], ptr);
    }
    return interfaceImpl;
  }

private:
  const mpc_state_t state_;
  const std::string name_;
  std::vector<identifier_t> inherits_;
  using name_t = std::optional<std::string>;
  std::vector<std::pair<Type, name_t>> functions_;

  inline static llvm::StringRef getName(llvm::Type* const interface) {
    constexpr static auto nameOffset = std::strlen("interface::");
    return interface->getStructName().substr(nameOffset);
  }
};

} // end namespace whack::ast

#endif // WHACK_INTERFACE_HPP
