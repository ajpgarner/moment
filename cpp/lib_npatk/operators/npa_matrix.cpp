/**
 * npa_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "npa_matrix.h"
#include "context.h"
#include "operator_sequence_generator.h"



namespace NPATK {
    NPAMatrix::NPAMatrix(const Context &the_context, size_t level)
        : context{the_context}, hierarchy_level{level}, UniqueSequences{*this} {
        OperatorSequenceGenerator colGen{context, hierarchy_level};
        OperatorSequenceGenerator rowGen{colGen.conjugate()};
        this->matrix_dimension = colGen.size();
        assert(this->matrix_dimension == rowGen.size());

        // Build matrix...
        this->matrix_data.reserve(this->matrix_dimension * this->matrix_dimension);
        for (const auto& rowSeq : rowGen) {
            for (const auto& colSeq : colGen) {
                this->matrix_data.emplace_back(rowSeq * colSeq);
            }
        }

        // Now, find unique elements (noting: complex conjugation...!)
        std::map<size_t, UniqueSequence> build_unique;
        std::map<size_t, size_t> conj_alias;

        for (size_t row = 0; row < this->matrix_dimension; ++row) {
            for (size_t col = row; col < this->matrix_dimension; ++col) {
                const auto& elem = this->matrix_data[(row*this->matrix_dimension) + col];
                const auto& conj_elem = this->matrix_data[(col*this->matrix_dimension) + row];
                bool hermitian = (elem == conj_elem);
                size_t hash = context.hash(elem);
                size_t conj_hash = hermitian ? hash : context.hash(conj_elem);

                if (hermitian) {
                    // Does exist?
                    if (!build_unique.contains(hash) && !conj_alias.contains(hash)) {
                        build_unique.emplace(hash, UniqueSequence{elem, hash});
                    }
                } else {
                    // Does exist?
                    if (!build_unique.contains(hash) && !conj_alias.contains(hash)) {
                        build_unique.emplace(hash, UniqueSequence{elem, hash, conj_elem, conj_hash});
                        conj_alias.emplace(conj_hash, hash);
                    }
                }
            }
        }

        // And flatten
        this->unique_sequences.reserve(build_unique.size());
        size_t count = 0;
        for (auto& [hash, elem] : build_unique) {
            bool hermitian = elem.hermitian;
            elem.id = count;
            this->unique_sequences.emplace_back(std::move(elem));
            this->fwd_hash_table.emplace_hint(this->fwd_hash_table.end(), hash, count);
            if (!hermitian) {
                this->conj_hash_table.emplace(this->unique_sequences[count].conj_hash, count);
            }
            ++count;
        }
    }

    const NPAMatrix::UniqueSequence *NPAMatrix::where(const OperatorSequence &seq) const noexcept {
        size_t hash = this->context.hash(seq);

        // Does this exist as its own symbol?
        auto find = this->fwd_hash_table.find(hash);
        if (find != this->fwd_hash_table.end()) {
            return &this->unique_sequences[find->second];
        }

        // Does this exist as a complex conjugate?
        auto find_alias = this->conj_hash_table.find(hash);
        if (find_alias != this->conj_hash_table.end()) {
            return &this->unique_sequences[find_alias->second];
        }

        // Doesn't exist anywhere
        return nullptr;
    }

}