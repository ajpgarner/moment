/**
 * full_monomial_specification.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include "utilities/iter_tuple.h"

#include "utilities/io_parameters.h"

#include "MatlabDataArray.hpp"

#include <complex>
#include <stdexcept>
#include <vector>

namespace Moment {
    struct Monomial;
    class OperatorSequence;
    class SymbolTable;
    namespace Inflation {
        class InflationContext;
    }
}

namespace Moment::mex {


    struct FullMonomialSpecification {
        const matlab::data::ArrayDimensions dimensions;
        const bool has_symbol_info;
        const bool has_alias_info;

        matlab::data::CellArray operators;
        matlab::data::TypedArray<std::complex<double>> coefficients;
        matlab::data::TypedArray<uint64_t> hashes;
        matlab::data::TypedArray<int64_t> symbol_ids;
        matlab::data::TypedArray<bool> is_conjugated;
        matlab::data::TypedArray<int64_t> real_basis_elems;
        matlab::data::TypedArray<int64_t> im_basis_elems;
        matlab::data::TypedArray<bool> is_aliased;

        using partial_iter_t =
            IterTuple<matlab::data::CellArray::iterator,
                     matlab::data::TypedArray<std::complex<double>>::iterator,
                     matlab::data::TypedArray<uint64_t>::iterator>;

        using full_iter_t =
            IterTuple<matlab::data::CellArray::iterator,
                    matlab::data::TypedArray<std::complex<double>>::iterator,
                    matlab::data::TypedArray<uint64_t>::iterator,
                    matlab::data::TypedArray<int64_t>::iterator,
                    matlab::data::TypedArray<bool>::iterator,
                    matlab::data::TypedArray<int64_t>::iterator,
                    matlab::data::TypedArray<int64_t>::iterator>;

        using full_with_alias_t =
            IterTuple<matlab::data::CellArray::iterator,
                    matlab::data::TypedArray<std::complex<double>>::iterator,
                    matlab::data::TypedArray<uint64_t>::iterator,
                    matlab::data::TypedArray<int64_t>::iterator,
                    matlab::data::TypedArray<bool>::iterator,
                    matlab::data::TypedArray<int64_t>::iterator,
                    matlab::data::TypedArray<int64_t>::iterator,
                    matlab::data::TypedArray<bool>::iterator>;


    public:
        FullMonomialSpecification(matlab::data::ArrayFactory& factory, size_t length,
                                  bool include_symbol_info, bool include_alias_info = false);

        FullMonomialSpecification(matlab::data::ArrayFactory& factory, matlab::data::ArrayDimensions dimensions,
                                  bool include_symbol_info, bool include_alias_info = false);

        FullMonomialSpecification(const FullMonomialSpecification& rhs) = delete;

        FullMonomialSpecification(FullMonomialSpecification&& rhs) noexcept = default;

        /** Move contents to output range. */
        void move_to_output(IOArgumentRange& output) noexcept;

        /** Move contents to a single cell array of constituent parts. */
        matlab::data::CellArray move_to_cell(matlab::data::ArrayFactory& factory);

        /** Write iter for data without symbol information. */
        partial_iter_t partial_write_begin();

        /** End of write iter for data without symbol information. */
        partial_iter_t partial_write_end();

        /** Write iter for full data */
        full_iter_t full_write_begin();

        /** End of write iter for full data */
        full_iter_t full_write_end();

        /** Write iter for full data with alias */
        full_with_alias_t full_with_alias_write_begin();

        /** End of write iter for full data with alias */
        full_with_alias_t full_with_alias_write_end();

    public:

        class missing_symbol_error : public std::runtime_error {
        public:
            explicit missing_symbol_error(const std::string& what) : std::runtime_error{what} { }
        public:
            [[nodiscard]] static missing_symbol_error make_from_seq(const OperatorSequence& missing);

            [[nodiscard]] static missing_symbol_error make_from_id(symbol_name_t id, symbol_name_t max);
        };

        /** Functors */
        class WriteFunctor {
        protected:
            matlab::data::ArrayFactory& factory;
            const SymbolTable& symbol_table;

        protected:
            WriteFunctor(matlab::data::ArrayFactory& factory, const SymbolTable& symbols)
                : factory{factory}, symbol_table{symbols} { }

        };

        /**
         * Write operator sequence of monomial.
         */
        class PartialWriteFunctor : public WriteFunctor {
        public:
            PartialWriteFunctor(matlab::data::ArrayFactory& factory, const SymbolTable& symbols)
                : WriteFunctor{factory, symbols} { }

            [[nodiscard]] typename partial_iter_t::value_type operator()(const Monomial& element) const;

            [[nodiscard]] typename partial_iter_t::value_type
            operator()(const OperatorSequence& sequence, std::complex<double> weight = {1.0, 0.0}) const;
        };


        /**
         * Write monomial.
         */
        class FullWriteFunctor : public WriteFunctor {
        public:
            FullWriteFunctor(matlab::data::ArrayFactory& factory, const SymbolTable& symbols)
                : WriteFunctor{factory, symbols} { }

            [[nodiscard]] typename full_iter_t::value_type operator()(const Monomial& element) const;

            [[nodiscard]] typename full_iter_t::value_type operator()(const OperatorSequence& sequence) const;

            [[nodiscard]] typename full_iter_t::value_type
            operator()(std::tuple<const Monomial&, const OperatorSequence&> mono_and_op) const;
        };

        /**
         * Write monomial, also detecting if an alias.
         */
        class AliasedWriteFunctor : public WriteFunctor {
        private:
            const Inflation::InflationContext& context;
        public:
            AliasedWriteFunctor(matlab::data::ArrayFactory& factory, const SymbolTable& symbols,
                                const Inflation::InflationContext& context)
                : WriteFunctor{factory, symbols}, context{context} { }

            /** Cannot infer if an alias from canonical symbol alone. */
            [[nodiscard]] typename full_with_alias_t::value_type operator()(const Monomial& element) const = delete;

            [[nodiscard]] typename full_with_alias_t::value_type operator()(const OperatorSequence& sequence) const;

            [[nodiscard]] typename full_with_alias_t::value_type
            operator()(std::tuple<const Monomial&, const OperatorSequence&, bool> mono_and_op) const;
        };
    };
}