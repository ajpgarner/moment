/**
 * full_monomial_specification.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "full_monomial_specification.h"

#include "export_operator_sequence.h"
#include "dictionary/operator_sequence.h"
#include "symbolic/symbol_table.h"

#include <sstream>

namespace Moment::mex {

    FullMonomialSpecification::FullMonomialSpecification(matlab::data::ArrayFactory &factory,
                                                         size_t length,
                                                         bool include_symbol_info)
            : dimensions({length, 1}), has_symbol_info{include_symbol_info},
              operators{factory.createCellArray(dimensions)},
              coefficients{factory.createArray<std::complex<double>>(dimensions)},
              hashes{factory.createArray<uint64_t>(dimensions)},
              symbol_ids{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                         : factory.createArray<int64_t>({0, 0})},
              is_conjugated{has_symbol_info ? factory.createArray<bool>(dimensions)
                                            : factory.createArray<bool>({0, 0})},
              real_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                               : factory.createArray<int64_t>({0, 0})},
              im_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                             : factory.createArray<int64_t>({0, 0})} {

    }


    FullMonomialSpecification::FullMonomialSpecification(matlab::data::ArrayFactory &factory,
                                                         matlab::data::ArrayDimensions array_dims,
                                                         bool include_symbol_info)
            : dimensions(std::move(array_dims)), has_symbol_info{include_symbol_info},
              operators{factory.createCellArray(dimensions)},
              coefficients{factory.createArray<std::complex<double>>(dimensions)},
              hashes{factory.createArray<uint64_t>(dimensions)},
              symbol_ids{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                         : factory.createArray<int64_t>({0, 0})},
              is_conjugated{has_symbol_info ? factory.createArray<bool>(dimensions)
                                            : factory.createArray<bool>({0, 0})},
              real_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                               : factory.createArray<int64_t>({0, 0})},
              im_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                             : factory.createArray<int64_t>({0, 0})} {

    }



    void FullMonomialSpecification::move_to_output(IOArgumentRange &output) noexcept {
        switch (output.size()) {
            default:
            case 7:
                output[6] = std::move(this->im_basis_elems);
            case 6:
                output[5] = std::move(this->real_basis_elems);
            case 5:
                output[4] = std::move(this->is_conjugated);
            case 4:
                output[3] = std::move(this->symbol_ids);
            case 3:
                output[2] = std::move(this->hashes);
            case 2:
                output[1] = std::move(this->coefficients);
            case 1:
                output[0] = std::move(this->operators);
            case 0:
                break;
        }
    }

    matlab::data::CellArray FullMonomialSpecification::move_to_cell(matlab::data::ArrayFactory &factory) {
        auto output = factory.createCellArray({1ULL, this->has_symbol_info ? 7ULL : 3ULL});
        output[0] = std::move(this->operators);
        output[1] = std::move(this->coefficients);
        output[2] = std::move(this->hashes);
        if (this->has_symbol_info) {
            output[3] = std::move(this->symbol_ids);
            output[4] = std::move(this->is_conjugated);
            output[5] = std::move(this->real_basis_elems);
            output[6] = std::move(this->im_basis_elems);
        }
        return output;
    }

    FullMonomialSpecification::partial_iter_t FullMonomialSpecification::partial_write_begin() {
        return partial_iter_t{this->operators.begin(), this->coefficients.begin(), this->hashes.begin()};
    }

    FullMonomialSpecification::partial_iter_t FullMonomialSpecification::partial_write_end() {
        return partial_iter_t{this->operators.end(), this->coefficients.end(), this->hashes.end()};
    }

    FullMonomialSpecification::full_iter_t FullMonomialSpecification::full_write_begin() {
        return full_iter_t{this->operators.begin(), this->coefficients.begin(), this->hashes.begin(),
                           this->symbol_ids.begin(), this->is_conjugated.begin(),
                           this->real_basis_elems.begin(), this->im_basis_elems.begin()};
    }

    FullMonomialSpecification::full_iter_t FullMonomialSpecification::full_write_end() {
        return full_iter_t{this->operators.end(), this->coefficients.end(), this->hashes.end(),
                           this->symbol_ids.end(), this->is_conjugated.end(),
                           this->real_basis_elems.end(), this->im_basis_elems.end()};
    }


    FullMonomialSpecification::missing_symbol_error
    FullMonomialSpecification::missing_symbol_error::make_from_seq(const OperatorSequence &missing) {
        std::stringstream errSS;
        errSS << "Could not find sequence '" << missing.formatted_string() << "' in symbol table.";
        return FullMonomialSpecification::missing_symbol_error(errSS.str());
    }

    FullMonomialSpecification::missing_symbol_error
    FullMonomialSpecification::missing_symbol_error::make_from_id(symbol_name_t id, symbol_name_t max) {
        std::stringstream errSS;
        errSS << "Symbol " << id << " is out of range (maximum symbol ID: " << max << ").";
        return FullMonomialSpecification::missing_symbol_error(errSS.str());
    }

    FullMonomialSpecification::partial_iter_t::value_type
    FullMonomialSpecification::PartialWriteFunctor::operator()(const Monomial& element) const {
        if ((element.id < 0) || (element.id >= this->symbol_table.size())) {
            throw missing_symbol_error::make_from_id(element.id,
                                                     static_cast<symbol_name_t>(this->symbol_table.size() - 1));
        }

        const auto& symbol = this->symbol_table[element.id];
        const auto& op_seq = element.conjugated ? symbol.sequence_conj() : symbol.sequence();
        return partial_iter_t::value_type {
                export_operator_sequence(this->factory, op_seq, true),
                element.factor,
                op_seq.hash()
        };
    }

    FullMonomialSpecification::partial_iter_t::value_type
    FullMonomialSpecification::PartialWriteFunctor::operator()(const OperatorSequence& sequence) const {
        return partial_iter_t::value_type {
            export_operator_sequence(this->factory, sequence, true),
            sequence.negated() ? -1.0 : 1.0,
            sequence.hash()
        };
    }

    FullMonomialSpecification::full_iter_t::value_type
    FullMonomialSpecification::FullWriteFunctor::operator()(const Monomial& element) const {
        if ((element.id < 0) || (element.id >= this->symbol_table.size())) {
            throw missing_symbol_error::make_from_id(element.id,
                                                     static_cast<symbol_name_t>(this->symbol_table.size() - 1));
        }
        const auto& symbol = this->symbol_table[element.id];
        const auto& op_seq = element.conjugated ? symbol.sequence_conj() : symbol.sequence();

        return full_iter_t::value_type {
                export_operator_sequence(this->factory, op_seq, true),
                element.factor,
                op_seq.hash(),
                element.id,
                element.conjugated,
                symbol.basis_key().first + 1,
                symbol.basis_key().second + 1
        };
    }

    FullMonomialSpecification::full_iter_t::value_type
    FullMonomialSpecification::FullWriteFunctor::operator()(const OperatorSequence& sequence) const {
        auto [symbol_ptr, entry_is_conjugated] = this->symbol_table.where_and_is_conjugated(sequence);
        if (nullptr == symbol_ptr) {
            throw missing_symbol_error::make_from_seq(sequence);
        }

        const auto& symbol = *symbol_ptr;
        return full_iter_t::value_type {
            export_operator_sequence(this->factory, sequence, true),
            sequence.negated() ? -1.0 : 1.0,
            sequence.hash(),
            symbol.Id(),
            entry_is_conjugated,
            symbol.basis_key().first + 1, // ML index
            symbol.basis_key().second +1  // ML index
        };
    }

    FullMonomialSpecification::full_iter_t::value_type
    FullMonomialSpecification::FullWriteFunctor::operator()(std::tuple<const Monomial&,
                                                                       const OperatorSequence&> input) const {
        const auto& monomial = std::get<0>(input);
        const auto& op_seq = std::get<1>(input);
        if ((monomial.id < 0) || (monomial.id >= this->symbol_table.size())) {
            throw missing_symbol_error::make_from_id(monomial.id,
                                                     static_cast<symbol_name_t>(this->symbol_table.size() - 1));
        }
        const auto& symbol_info = this->symbol_table[monomial.id];

        return full_iter_t::value_type{
                export_operator_sequence(factory, op_seq, true), // ML indexing
                monomial.factor,
                op_seq.hash(),
                monomial.id,
                monomial.conjugated,
                symbol_info.basis_key().first + 1, // ML indexing
                symbol_info.basis_key().second + 1 // ML indexing
        };
    }
}