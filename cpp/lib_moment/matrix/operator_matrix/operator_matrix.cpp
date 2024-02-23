/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"
#include "operator_matrix_transformation.h"

#include "dictionary/operator_sequence_generator.h"
#include "dictionary/raw_polynomial.h"

#include "symbolic/monomial.h"
#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"

#include <limits>
#include <stdexcept>

namespace Moment {

    namespace {

        template<bool premultiply>
        inline std::vector<std::unique_ptr<OperatorMatrix>>
        do_poly_multiply(const OperatorMatrix& the_matrix, const Polynomial& the_poly,
                         const SymbolTable &symbols,
                         Multithreading::MultiThreadPolicy policy)  {
            std::vector<std::unique_ptr<OperatorMatrix>> output;
            for (const auto& monomial : the_poly) {
                assert(monomial.id < symbols.size());
                const auto& symbolic_resolution = symbols[monomial.id];
                assert(symbolic_resolution.has_sequence());
                const auto& sequence = monomial.conjugated ? symbolic_resolution.sequence_conj()
                                                           : symbolic_resolution.sequence();
                if constexpr(premultiply) {
                    output.emplace_back(the_matrix.pre_multiply(sequence, policy));
                } else {
                    output.emplace_back(the_matrix.post_multiply(sequence, policy));
                }
            }
            return output;
        }

    }

   OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)) {
       this->non_hermitian_elem = NonHInfo::find_first_index(*this);
       this->hermitian = !this->non_hermitian_elem.has_value();
    }

    OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data,
                                             std::optional<NonHInfo> h_info)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)), non_hermitian_elem{std::move(h_info)} {
        this->hermitian = !non_hermitian_elem.has_value();
    }

    OperatorMatrix::OperatorMatrix(const Context& context, std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
       : context{context},  op_seq_matrix{std::move(op_seq_mat)} {
           assert(op_seq_matrix);
       }

    OperatorMatrix::~OperatorMatrix() noexcept = default;

    std::unique_ptr<OperatorMatrix> OperatorMatrix::clone(Multithreading::MultiThreadPolicy policy) const {
        const size_t dimension = this->Dimension();

        std::vector<OperatorSequence> cloned_data;
        cloned_data.reserve(dimension * dimension);
        std::copy(this->op_seq_matrix->begin(), this->op_seq_matrix->end(), std::back_inserter(cloned_data));

        return std::make_unique<OperatorMatrix>(this->context,
                        std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(cloned_data)));
    }

    const OSGPair& OperatorMatrix::generators() const {
        throw std::runtime_error{"Generic OperatorMatrix does not have any attached generators."};
    }

    void OperatorMatrix::set_properties(SymbolicMatrix &matrix) const {
        matrix.description = this->description();
        matrix.hermitian = this->is_hermitian();
    }

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::pre_multiply(const OperatorSequence& lhs, Multithreading::MultiThreadPolicy policy) const {
        assert(lhs.is_same_context(this->context));
        OperatorMatrixTransformation pre_multiplier{
            [&lhs](const OperatorSequence& matrix_elem) -> OperatorSequence {
            return lhs * matrix_elem;
        }, policy, Multithreading::minimum_matrix_multiply_element_count};

        return pre_multiplier(*this);
    }

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::post_multiply(const OperatorSequence& rhs, Multithreading::MultiThreadPolicy policy) const {
        OperatorMatrixTransformation post_multiplier{
            [&rhs](const OperatorSequence& matrix_elem) -> OperatorSequence {
            return matrix_elem * rhs;
        }, policy, Multithreading::minimum_matrix_multiply_element_count};

        return post_multiplier(*this);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::pre_multiply(const RawPolynomial& lhs, Multithreading::MultiThreadPolicy policy) const {
        // NB: For now, multithreaded approach is to generate each constituent matrix in a multithreaded manner.
        // It is plausible that this is slower than multithreading by distributing each monomial to a different thread
        // to generate in a single-threaded manner. This former approach is consistent with how multithreaded actions
        // work on matrices in general in Moment, so unless profiling reveals this to be a bottleneck, I am not
        // motivated to switch.
        std::vector<std::unique_ptr<OperatorMatrix>> output;
        for (const auto& monomial : lhs) {
            output.emplace_back(pre_multiply(monomial.sequence, policy));
        }
        return output;
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::post_multiply(const RawPolynomial& rhs, Multithreading::MultiThreadPolicy policy) const {
        std::vector<std::unique_ptr<OperatorMatrix>> output;
        for (const auto& monomial : rhs) {
            output.emplace_back(this->post_multiply(monomial.sequence, policy));
        }
        return output;
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::pre_multiply(const Polynomial &lhs, const SymbolTable &symbols,
                                 Multithreading::MultiThreadPolicy policy) const {
        return do_poly_multiply<true>(*this, lhs, symbols, policy);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::post_multiply(const Polynomial &rhs, const SymbolTable &symbols,
                                  Multithreading::MultiThreadPolicy policy) const {
        return do_poly_multiply<false>(*this, rhs, symbols, policy);
    }

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::simplify_as_moments(Multithreading::MultiThreadPolicy policy) const {
        if (!this->context.can_have_aliases()) {
            return nullptr;
        }

        const Context& the_context = this->context;
        OperatorMatrixTransformation simplifier{
                [&the_context](const OperatorSequence& matrix_elem) -> OperatorSequence {
                    return the_context.simplify_as_moment(matrix_elem);
                }, policy, Multithreading::minimum_matrix_element_count};

        return simplifier(*this);
    }

}