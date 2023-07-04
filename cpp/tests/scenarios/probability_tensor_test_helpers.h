/**
 * probability_tensor_test_helpers.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "scenarios/probability_tensor.h"

#include <string>
#include <span>


namespace Moment::Tests {
    void testIdMmt(const ProbabilityTensor::ProbabilityTensorRange& spanID);

    void testSingleCV(const ProbabilityTensor::ProbabilityTensorRange& range,
                      symbol_name_t id,
                      const std::string& ctx = "");

    void test2Mmt(const ProbabilityTensor::ProbabilityTensorRange& spanA,
                  symbol_name_t id, symbol_name_t alice,
                  const std::string& ctx = "");

    void test3Mmt(const ProbabilityTensor::ProbabilityTensorRange& spanA,
                  symbol_name_t id, symbol_name_t a0, symbol_name_t a1,
                  const std::string& ctx = "");

    void test22JoinMmt(const ProbabilityTensor::ProbabilityTensorRange& spanAB,
                       symbol_name_t id,
                       symbol_name_t alice,
                       symbol_name_t bob,
                       symbol_name_t alice_bob,
                       const std::string& ctx = "");

    void test32JoinMmt(const ProbabilityTensor::ProbabilityTensorRange& spanAB,
                       symbol_name_t id,
                       symbol_name_t a0,
                       symbol_name_t a1,
                       symbol_name_t b,
                       symbol_name_t a0b,
                       symbol_name_t a1b,
                       const std::string& ctx = "");

    void test222JoinMmt(const ProbabilityTensor::ProbabilityTensorRange& spanABC,
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