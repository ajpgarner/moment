/**
 * implicit_symbol_test_helpers.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operators/common/implicit_symbols.h"

#include <string>
#include <span>

namespace NPATK::Tests {
    void test2Mmt(std::span<const PMODefinition> spanA,
                  symbol_name_t id, symbol_name_t alice,
                  const std::string& ctx = "");

    void test3Mmt(std::span<const PMODefinition> spanA,
                  symbol_name_t id, symbol_name_t a0, symbol_name_t a1,
                  const std::string& ctx = "");

    void test22JoinMmt(std::span<const PMODefinition> spanAB,
                       symbol_name_t id,
                       symbol_name_t alice,
                       symbol_name_t bob,
                       symbol_name_t alice_bob,
                       const std::string& ctx = "");

    void test32JoinMmt(std::span<const PMODefinition> spanAB,
                       symbol_name_t id,
                       symbol_name_t a0,
                       symbol_name_t a1,
                       symbol_name_t b,
                       symbol_name_t a0b,
                       symbol_name_t a1b,
                       const std::string& ctx = "");

    void test222JoinMmt(std::span<const PMODefinition> spanABC,
                        symbol_name_t id,
                        symbol_name_t alice,
                        symbol_name_t bob,
                        symbol_name_t charlie,
                        symbol_name_t alice_bob,
                        symbol_name_t alice_charlie,
                        symbol_name_t bob_charlie,
                        symbol_name_t alice_bob_charlie,
                        const std::string& ctx = "");
}