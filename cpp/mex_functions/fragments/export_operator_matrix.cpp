/**
 * exported_symbol_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "export_operator_matrix.h"

#include "matrix_system.h"
#include "matrix/matrix_properties.h"
#include "matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"
#include "scenarios/context.h"
#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/reporting.h"

#include "error_codes.h"

#include "mex.hpp"

namespace Moment::mex {

    namespace {
        class DirectFormatView {
        public:
            using raw_const_iterator = SquareMatrix<OperatorSequence>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context* context = nullptr;
                DirectFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Context& context, raw_const_iterator rci)
                        : context{&context}, raw_iter{rci} { }


                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);
                    return {matlab::engine::convertUTF8StringToUTF16String(context->format_sequence(*raw_iter))};
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            DirectFormatView(const Context &context, const SquareMatrix<OperatorSequence>& inputMatrix)
                : iter_begin{context, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };

        class InferredFormatView {
        public:
            using raw_const_iterator = SquareMatrix<SymbolExpression>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context * context = nullptr;
                const SymbolTable * symbols = nullptr;
                InferredFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Context& context,
                                         const SymbolTable& symbols,
                                         raw_const_iterator rci)
                        : context{&context}, symbols{&symbols}, raw_iter{rci} { }

                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);

                    const auto id = raw_iter->id;
                    if ((raw_iter->id < 0) || (raw_iter->id >= symbols->size())) {
                        std::stringstream ssErr;
                        ssErr << "[MISSING:" << id << "]";
                        return {matlab::engine::convertUTF8StringToUTF16String(ssErr.str())};
                    }
                    const auto& symEntry = (*symbols)[raw_iter->id];
                    const std::string symbol_str = context->format_sequence(symEntry.sequence());

                    if (1.0 == raw_iter->factor) {
                        return {matlab::engine::convertUTF8StringToUTF16String(symbol_str)};
                    }
                    std::stringstream ss;
                    if (-1.0 == raw_iter->factor) {
                        ss << "-" << symbol_str;
                    } else {
                        ss << raw_iter->factor;
                        if (symEntry.Id() != 1) {
                            ss << symbol_str;
                        }
                    }

                    return {matlab::engine::convertUTF8StringToUTF16String(ss.str())};

                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            InferredFormatView(const Context &context, const SymbolTable& symbols,
                             const SquareMatrix<SymbolExpression>& inputMatrix)
                : iter_begin{context, symbols, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, symbols, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };

        class FactorFormatView {
        public:
            using raw_const_iterator = SquareMatrix<SymbolExpression>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Inflation::InflationContext * context = nullptr;
                const Inflation::FactorTable * factors = nullptr;
                FactorFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Inflation::InflationContext& context,
                                         const Inflation::FactorTable& factors,
                                         raw_const_iterator rci)
                        : context{&context}, factors{&factors}, raw_iter{rci} { }

                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);

                    const auto id = raw_iter->id;
                    if ((raw_iter->id < 0) || (raw_iter->id >= factors->size())) {
                        std::stringstream ssErr;
                        ssErr << "[MISSING:" << id << "]";
                        return {matlab::engine::convertUTF8StringToUTF16String(ssErr.str())};
                    }
                    const auto& facEntry = (*factors)[raw_iter->id];
                    if (1.0 == raw_iter->factor) {
                        return {matlab::engine::convertUTF8StringToUTF16String(facEntry.sequence_string())};
                    }
                    std::stringstream ss;
                    if (-1.0 == raw_iter->factor) {
                        ss << "-" << facEntry.sequence_string();
                    } else {
                        ss << raw_iter->factor;
                        if (facEntry.id != 1) {
                            ss << facEntry.sequence_string();
                        }
                    }

                    return {matlab::engine::convertUTF8StringToUTF16String(ss.str())};

                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            FactorFormatView(const Inflation::InflationContext &context, const Inflation::FactorTable& factors,
                             const SquareMatrix<SymbolExpression>& inputMatrix)
                : iter_begin{context, factors, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, factors, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };
    }

    matlab::data::Array export_symbol_matrix(matlab::engine::MATLABEngine& engine,
                                             const SquareMatrix<SymbolExpression>& inputMatrix) {
        matlab::data::ArrayFactory factory;
        matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};

        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
        auto writeIter = outputArray.begin();

        auto readIter = inputMatrix.ColumnMajor.begin();

        while ((writeIter != outputArray.end()) && (readIter != inputMatrix.ColumnMajor.end())) {
            *writeIter = readIter->as_string();
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix count_indices mismatch: too few input elements." );
        }
        if (readIter != inputMatrix.ColumnMajor.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix count_indices mismatch: too many input elements.");
        }

        return outputArray;
    }

    matlab::data::Array
    export_sequence_matrix(matlab::engine::MATLABEngine &engine,
                            const Context &context,
                            const SquareMatrix<OperatorSequence>& inputMatrix) {
        matlab::data::ArrayFactory factory;
        matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};

        DirectFormatView formatView{context, inputMatrix};

        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
        auto writeIter = outputArray.begin();
        auto readIter = formatView.begin();

        while ((writeIter != outputArray.end()) && (readIter != formatView.end())) {
            *writeIter = *readIter;
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix index count mismatch: too few input elements." );
        }
        if (readIter != formatView.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix index count mismatch: too many input elements.");
        }

        return outputArray;
    }

    matlab::data::Array
    export_inferred_sequence_matrix(matlab::engine::MATLABEngine& engine,
                                  const Context& context,
                                  const SymbolTable& symbols,
                                  const SymbolicMatrix& inputMatrix) {
        matlab::data::ArrayFactory factory;

        // Prepare output
        const auto dimension = inputMatrix.Dimension();
        matlab::data::ArrayDimensions array_dims{dimension, dimension};
        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));

        InferredFormatView formatView{context, symbols, inputMatrix.SymbolMatrix()};

        auto writeIter = outputArray.begin();
        auto readIter = formatView.begin();

        while ((writeIter != outputArray.end()) && (readIter != formatView.end())) {
            *writeIter = *readIter;
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_factor_sequence_matrix  index count mismatch: too few input elements." );
        }
        if (readIter != formatView.end()) {
            throw_error(engine, errors::internal_error,
                        "export_factor_sequence_matrix index count mismatch: too many input elements.");
        }
        return outputArray;
    }

    matlab::data::Array
    export_factor_sequence_matrix(matlab::engine::MATLABEngine& engine,
                                  const Inflation::InflationContext& context,
                                  const Inflation::FactorTable& factors,
                                  const SymbolicMatrix& inputMatrix) {
        matlab::data::ArrayFactory factory;

        // Prepare output
        const auto dimension = inputMatrix.Dimension();
        matlab::data::ArrayDimensions array_dims{dimension, dimension};
        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));

        FactorFormatView formatView{context, factors, inputMatrix.SymbolMatrix()};

        auto writeIter = outputArray.begin();
        auto readIter = formatView.begin();

        while ((writeIter != outputArray.end()) && (readIter != formatView.end())) {
            *writeIter = *readIter;
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_factor_sequence_matrix  index count mismatch: too few input elements." );
        }
        if (readIter != formatView.end()) {
            throw_error(engine, errors::internal_error,
                        "export_factor_sequence_matrix index count mismatch: too many input elements.");
        }
        return outputArray;
    }

    matlab::data::Array
    export_sequence_matrix(matlab::engine::MATLABEngine &engine, const MatrixSystem& system,
                           const SymbolicMatrix &matrix) {

        // Is this an inflation matrix? If so, display factorized format:
        const auto* inflSystem = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
        if (nullptr != inflSystem) {
            return export_factor_sequence_matrix(engine, inflSystem->InflationContext(), inflSystem->Factors(), matrix);
        }

        // Attempt to use normal context formatting
        const auto* opMatPtr = dynamic_cast<const Moment::OperatorMatrix*>(&matrix);
        if (nullptr != opMatPtr) {
            return export_sequence_matrix(engine, opMatPtr->context, opMatPtr->SequenceMatrix());
        }

        // If all else fails, use inferred string formatting
        return export_inferred_sequence_matrix(engine, system.Context(), system.Symbols(), matrix);
    }
}
