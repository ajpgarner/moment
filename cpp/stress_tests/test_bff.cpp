/**
 * test_bff.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "test_bff.h"

#include "dictionary/operator_sequence_generator.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "dictionary/operator_sequence.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include <chrono>
#include <iostream>
#include <ratio>


namespace Moment::StressTests {
    using namespace Moment::Algebraic;

    BrownFawziFawzi::BrownFawziFawzi(const size_t M, const size_t mm_level) 
        : M{M}, mm_level{mm_level} {

    }

    void BrownFawziFawzi::set_up_ams() {

        const oper_name_t z_ops = 2*this->M; // One z for a0, one for (1-a0)...

        AlgebraicPrecontext apc{static_cast<oper_name_t>(4+z_ops), AlgebraicPrecontext::ConjugateMode::Interleaved};

        const oper_name_t expected_rule_count = 12 + z_ops*8;

        std::vector<OperatorRule> rules;
        rules.reserve(expected_rule_count);

        for (oper_name_t raw_op = 0; raw_op < 4; ++raw_op) {
            const oper_name_t op = raw_op * 2;
            // Hermitian
            rules.emplace_back(
                    HashedSequence{{static_cast<oper_name_t>(op+1)}, apc.hasher},
                    HashedSequence{{op}, apc.hasher}
            );

            // Projective:
            rules.emplace_back(
                    HashedSequence{{op, op}, apc.hasher},
                    HashedSequence{{op}, apc.hasher}
            );
        }

        // Commutation [Ai, Bi]
        rules.emplace_back(
                HashedSequence{{4, 0}, apc.hasher},
                HashedSequence{{0, 4}, apc.hasher}
        );
        rules.emplace_back(
                HashedSequence{{6, 0}, apc.hasher},
                HashedSequence{{0, 6}, apc.hasher}
        );
        rules.emplace_back(
                HashedSequence{{4, 2}, apc.hasher},
                HashedSequence{{2, 4}, apc.hasher}
        );
        rules.emplace_back(
                HashedSequence{{6, 2}, apc.hasher},
                HashedSequence{{2, 6}, apc.hasher}
        );

        // Commutation, Z_i with everything
        for (oper_name_t z_index = 0; z_index < z_ops; ++z_index) {
            const oper_name_t z_op = 8 + z_index*2;
            const oper_name_t z_op_star = z_op+1;
            for (oper_name_t local_index = 0; local_index < 4; ++local_index) {
                const oper_name_t local_op = local_index * 2;
                rules.emplace_back(
                        HashedSequence{{z_op, local_op}, apc.hasher},
                        HashedSequence{{local_op, z_op}, apc.hasher}
                );
                rules.emplace_back(
                        HashedSequence{{z_op_star, local_op}, apc.hasher},
                        HashedSequence{{local_op, z_op_star}, apc.hasher}
                );
            }
        }


        this->ams_ptr = std::make_unique<AlgebraicMatrixSystem>(
                std::make_unique<AlgebraicContext>(apc, false, false, std::move(rules))
        );
        const auto& context = this->ams_ptr->AlgebraicContext();
        bool complete = context.rulebook().is_complete();
        if (!complete) {
            throw std::logic_error{"Not every expected rule was created."};
        }
    }

    void BrownFawziFawzi::make_moment_matrix() {
        auto& mm = this->ams_ptr->MomentMatrix(2);
        this->moment_matrix = &mm;
    }

}

int main() {
    using namespace Moment::StressTests;
    BrownFawziFawzi bff{3, 2};

    std::cout << "Setting up matrix system..." << std::endl;
    const auto before_ams = std::chrono::high_resolution_clock::now();
    bff.set_up_ams();
    const auto done_ams = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ams_duration = done_ams - before_ams;
    std::cout << "... done in " << ams_duration << "." << std::endl;

    std::cout << "Generating moment matrix level " << bff.mm_level << "..." << std::endl;
    const auto before_mm = std::chrono::high_resolution_clock::now();
    bff.make_moment_matrix();
    const auto done_mm = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> mm_duration = done_mm - before_mm;

    std::cout << "... done in " << mm_duration << "." << std::endl;

    return 0;
}

