/**
 * exported_symbol_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "export_symbol_matrix.h"

#include "error_codes.h"
#include "operators/context.h"
#include "utilities/reporting.h"

#include "mex.hpp"

namespace NPATK::mex {

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
                        "export_symbol_matrix dimension mismatch: too few input elements." );
        }
        if (readIter != inputMatrix.ColumnMajor.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix dimension mismatch: too many input elements.");
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
                        "export_symbol_matrix dimension mismatch: too few input elements." );
        }
        if (readIter != formatView.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix dimension mismatch: too many input elements.");
        }

        return outputArray;
    }

}
