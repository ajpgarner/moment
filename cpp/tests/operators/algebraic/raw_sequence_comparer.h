/**
 * raw_sequence_comparer.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/algebraic/algebraic_matrix_system.h"
#include "operators/algebraic/algebraic_context.h"
#include "operators/algebraic/raw_sequence_book.h"


namespace NPATK::Tests {

    class RawSequenceComparer {
    private:
        const AlgebraicContext& context;
        const RawSequenceBook& book;

    public:
        RawSequenceComparer(const AlgebraicContext& the_context, const RawSequenceBook& the_book)
            : context{the_context}, book{the_book} { }

        void find_and_compare(std::vector<oper_name_t> op_seq, symbol_name_t expected_symbol) const;
        void find_and_compare_zero() const;
        void find_and_compare_id() const;

    };

}