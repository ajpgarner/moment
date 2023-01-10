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
        class FormatView {
        public:
            using raw_const_iterator = SquareMatrix<OperatorSequence>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context* context = nullptr;
                FormatView::raw_const_iterator raw_iter;

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

            static_assert(std::input_iterator<FormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            FormatView(const Context &context, const SquareMatrix<OperatorSequence>& inputMatrix)
                : iter_begin{context, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, inputMatrix.ColumnMajor.end()} {
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
                    return {matlab::engine::convertUTF8StringToUTF16String(facEntry.sequence_string())};
                }
            };

            static_assert(std::input_iterator<FormatView::const_iterator>);

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

        FormatView formatView{context, inputMatrix};

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

        // Otherwise, just use normal context formatting
        const auto* opMatPtr = dynamic_cast<const Moment::OperatorMatrix*>(&matrix);
        if (nullptr != opMatPtr) {
            return export_sequence_matrix(engine, opMatPtr->context, opMatPtr->SequenceMatrix());
        }

        throw_error(engine, errors::internal_error,
                    "Could not access sequence matrix information for requested matrix.");
    }


    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<uint64_t>>
    export_basis_lists(matlab::engine::MATLABEngine &engine, const MatrixProperties &smp) {
        // Prepare output lists
        matlab::data::ArrayFactory factory;
        auto output = std::make_pair(
                factory.createArray<uint64_t>(matlab::data::ArrayDimensions{1, smp.RealSymbols().size()}),
                factory.createArray<uint64_t>(matlab::data::ArrayDimensions{1, smp.ImaginarySymbols().size()})
                );

        // Copy real symbols
        auto re_write_iter = output.first.begin();
        for (auto sym : smp.RealSymbols()) {
            *re_write_iter = sym;
            ++re_write_iter;
        }

        // Copy imaginary symbols
        if (!smp.ImaginarySymbols().empty()) {
            auto im_write_iter = output.second.begin();
            for (auto sym: smp.ImaginarySymbols()) {
                *im_write_iter = sym;
                ++im_write_iter;
            }
        }

        return output;
    }


    std::pair<matlab::data::TypedArray<bool>, matlab::data::TypedArray<bool>>
    export_basis_masks(matlab::engine::MATLABEngine& engine,
                       const SymbolTable& symbol_table,
                       const MatrixProperties& smp) {

        // Prepare masks
        matlab::data::ArrayFactory factory;
        auto output = std::make_pair(
                factory.createArray<bool>(matlab::data::ArrayDimensions{1, symbol_table.size()-1}),
                factory.createArray<bool>(matlab::data::ArrayDimensions{1, symbol_table.size()-1})
        );

        // Get read iterators, skipping symbol 0
        auto re_read_iter = smp.RealSymbols().begin();
        const auto re_read_end = smp.RealSymbols().end();
        if ((re_read_iter != re_read_end) && (*re_read_iter == 0)) {
            ++re_read_iter;
        }

        auto im_read_iter = smp.ImaginarySymbols().begin();
        const auto im_read_end = smp.ImaginarySymbols().end();
        if ((im_read_iter != im_read_end) && (*im_read_iter == 0)) {
            ++im_read_iter;
        }

        for (size_t n = 1, nMax = symbol_table.size(); n < nMax; ++n) {
            if ((re_read_iter != re_read_end) && (n == *re_read_iter)) {
                output.first[n-1] = true;
                ++re_read_iter;
            } else {
                output.first[n-1] = false;
            }

            if ((im_read_iter != im_read_end) && (n == *im_read_iter)) {
                output.second[n-1] = true;
                ++im_read_iter;
            } else {
                output.second[n-1] = false;
            }
        }

        return output;
    }

    matlab::data::TypedArray<int32_t>
    export_basis_key(matlab::engine::MATLABEngine &engine, const MatrixProperties &imp) {
        matlab::data::ArrayFactory factory{};
        matlab::data::ArrayDimensions dims{imp.BasisKey().size(),
                                           (imp.Type() == MatrixType::Hermitian) ? 3U : 2U};
        matlab::data::TypedArray<int32_t> output = factory.createArray<int32_t>(dims);

        if (imp.Type() == MatrixType::Hermitian) {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: imp.BasisKey()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                output[index][2] = static_cast<int32_t>(re_im_pair.second) + 1;
                ++index;
            }
        } else {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: imp.BasisKey()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                ++index;
            }
        }


        return output;
    }

}
