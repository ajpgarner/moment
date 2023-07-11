/**
 * export_sequence_matrix.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_operator_matrix_seq_strings.h"

#include "matrix_system.h"
#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "symbolic/symbol_table.h"
#include "scenarios/context.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/reporting.h"

#include "utilities/format_factor.h"
#include "utilities/utf_conversion.h"

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
                const Context *context = nullptr;
                DirectFormatView::raw_const_iterator raw_iter;
                UTF8toUTF16Convertor convertor;

            public:
                constexpr const_iterator(const Context &context, raw_const_iterator rci)
                        : context{&context}, raw_iter{rci} {}


                constexpr bool operator==(const const_iterator &rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator &rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator &operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) &{
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);
                    return {convertor(context->format_sequence(*raw_iter))};
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            DirectFormatView(const Context &context, const SquareMatrix<OperatorSequence> &inputMatrix)
                    : iter_begin{context, inputMatrix.ColumnMajor.begin()},
                      iter_end{context, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }

            [[nodiscard]] auto end() const { return iter_end; }

        };

        class LocalityFormatView {
        public:
            using raw_const_iterator = SquareMatrix<OperatorSequence>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Locality::LocalityContext *context = nullptr;
                const Locality::LocalityOperatorFormatter *formatter = nullptr;
                DirectFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Locality::LocalityContext &context,
                                         const Locality::LocalityOperatorFormatter &formatter,
                                         raw_const_iterator rci)
                        : context{&context}, formatter{&formatter}, raw_iter{rci} {}


                constexpr bool operator==(const const_iterator &rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator &rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator &operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) &{
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);
                    assert(formatter != nullptr);
                    return {UTF8toUTF16Convertor::convert(
                            context->format_sequence(*formatter, *raw_iter)
                    )};
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;
            const Locality::LocalityOperatorFormatter &formatter;

        public:
            LocalityFormatView(const Locality::LocalityContext &context,
                               const Locality::LocalityOperatorFormatter &formatter,
                               const SquareMatrix<OperatorSequence> &inputMatrix)
                    : iter_begin{context, formatter, inputMatrix.ColumnMajor.begin()},
                      iter_end{context, formatter, inputMatrix.ColumnMajor.end()}, formatter{formatter} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }

            [[nodiscard]] auto end() const { return iter_end; }

        };

        class InferredFormatView {
        public:
            using raw_const_iterator = SquareMatrix<Monomial>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context *context = nullptr;
                const SymbolTable *symbols = nullptr;
                InferredFormatView::raw_const_iterator raw_iter;


            public:
                constexpr const_iterator(const Context &context,
                                         const SymbolTable &symbols,
                                         raw_const_iterator rci)
                        : context{&context}, symbols{&symbols}, raw_iter{rci} {}

                constexpr bool operator==(const const_iterator &rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator &rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator &operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) &{
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    return {UTF8toUTF16Convertor::convert(infer_one_symbol(*symbols, *raw_iter))};
                }

                [[nodiscard]] static std::string infer_one_symbol(const SymbolTable &symbols,
                                                                  const Monomial &expr,
                                                                  bool with_prefix = false) {

                    std::stringstream ss;
                    if ((expr.id < 0) || (expr.id >= symbols.size())) {
                        if (with_prefix) {
                            ss << " + ";
                        }
                        ss << "[MISSING:" << expr.id << "]";
                        return ss.str();
                    }

                    const auto &symEntry = symbols[expr.id];

                    std::string symbol_str = expr.conjugated ? symEntry.formatted_sequence_conj()
                                                             : symEntry.formatted_sequence();

                    if (!approximately_zero(expr.factor)) {
                        const bool is_scalar = (symEntry.Id() == 1);
                        const bool need_space = format_factor(ss, expr.factor, is_scalar, with_prefix);
                        if (symEntry.Id() != 1) {
                            if (need_space) {
                                ss << " ";
                            }
                            ss << symbol_str;
                        }
                    } else {
                        if (with_prefix) {
                            return "";
                        } else {
                            return "0";
                        }
                    }
                    return ss.str();
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            InferredFormatView(const Context &context, const SymbolTable &symbols,
                               const SquareMatrix<Monomial> &inputMatrix)
                    : iter_begin{context, symbols, inputMatrix.ColumnMajor.begin()},
                      iter_end{context, symbols, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }

            [[nodiscard]] auto end() const { return iter_end; }

        };

        class InferredPolynomialFormatView {
        public:
            using raw_const_iterator = SquareMatrix<Polynomial>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context *context = nullptr;
                const SymbolTable *symbols = nullptr;
                InferredPolynomialFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Context &context,
                                         const SymbolTable &symbols,
                                         raw_const_iterator rci)
                        : context{&context}, symbols{&symbols}, raw_iter{rci} {}

                constexpr bool operator==(const const_iterator &rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator &rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator &operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) &{
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    bool done_once = false;
                    std::stringstream output;
                    for (const auto &expr: *raw_iter) {
                        output << InferredFormatView::const_iterator::infer_one_symbol(*symbols, expr, done_once);
                        done_once = true;
                    }
                    return {UTF8toUTF16Convertor::convert(output.str())};
                }
            };

            static_assert(std::input_iterator<InferredPolynomialFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            InferredPolynomialFormatView(const Context &context, const SymbolTable &symbols,
                                         const SquareMatrix<Polynomial> &inputMatrix)
                    : iter_begin{context, symbols, inputMatrix.ColumnMajor.begin()},
                      iter_end{context, symbols, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }

            [[nodiscard]] auto end() const { return iter_end; }

        };


        class FactorFormatView {
        public:
            using raw_const_iterator = SquareMatrix<Monomial>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Inflation::InflationContext *context = nullptr;
                const Inflation::FactorTable *factors = nullptr;
                FactorFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Inflation::InflationContext &context,
                                         const Inflation::FactorTable &factors,
                                         raw_const_iterator rci)
                        : context{&context}, factors{&factors}, raw_iter{rci} {}

                constexpr bool operator==(const const_iterator &rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator &rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator &operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) &{
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
                        return {UTF8toUTF16Convertor::convert(ssErr.str())};
                    }
                    if (raw_iter->id == 0) {
                        return {u"0"};
                    }

                    const auto &facEntry = (*factors)[raw_iter->id];
                    if (approximately_equal(raw_iter->factor, 1.0)) {
                        return {UTF8toUTF16Convertor::convert(facEntry.sequence_string())};
                    }

                    std::stringstream ss;
                    const bool is_scalar = (facEntry.id == 1);
                    const bool need_space = format_factor(ss, raw_iter->factor, is_scalar, false);
                    if (!is_scalar) {
                        if (need_space) {
                            ss << " ";
                        }
                        ss << facEntry.sequence_string();
                    }

                    return {UTF8toUTF16Convertor::convert(ss.str())};

                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            FactorFormatView(const Inflation::InflationContext &context, const Inflation::FactorTable &factors,
                             const SquareMatrix<Monomial> &inputMatrix)
                    : iter_begin{context, factors, inputMatrix.ColumnMajor.begin()},
                      iter_end{context, factors, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }

            [[nodiscard]] auto end() const { return iter_end; }

        };


        template<class format_view_t, class matrix_data_t, typename... Args>
        matlab::data::Array do_export(matlab::engine::MATLABEngine &engine,
                                      const SquareMatrix<matrix_data_t> &inputMatrix,
                                      Args &... fv_extra_args) {

            matlab::data::ArrayFactory factory;
            matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};

            format_view_t formatView{std::forward<Args &>(fv_extra_args)..., inputMatrix};

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
                            "export_symbol_matrix index count mismatch: too few input elements.");
            }
            if (readIter != formatView.end()) {
                throw_error(engine, errors::internal_error,
                            "export_symbol_matrix index count mismatch: too many input elements.");
            }

            return outputArray;
        }

        inline matlab::data::StringArray export_direct(matlab::engine::MATLABEngine& engine,
                                                       const OperatorMatrix &opMatrix)  {
            return do_export<DirectFormatView>(engine, opMatrix(), opMatrix.context);
        }

        inline matlab::data::StringArray export_inferred(matlab::engine::MATLABEngine& engine,
                                                         const MonomialMatrix &inputMatrix) {
            return do_export<InferredFormatView>(engine, inputMatrix.SymbolMatrix(),
                                                 inputMatrix.context, inputMatrix.symbols);
        }

        inline matlab::data::StringArray export_inferred(matlab::engine::MATLABEngine& engine,
                                                         const PolynomialMatrix &inputMatrix) {
            return do_export<InferredPolynomialFormatView>(engine, inputMatrix.SymbolMatrix(),
                                                           inputMatrix.context, inputMatrix.symbols);
        }

        inline matlab::data::StringArray export_factored(matlab::engine::MATLABEngine& engine,
                                                         const Inflation::InflationMatrixSystem& ims,
                                                         const MonomialMatrix &inputMatrix)  {

            return do_export<FactorFormatView>(engine, inputMatrix.SymbolMatrix(),
                                               ims.InflationContext(), ims.Factors());
        }

        inline matlab::data::StringArray export_locality(matlab::engine::MATLABEngine& engine,
                                                         const Locality::LocalityContext& context,
                                                         const Locality::LocalityOperatorFormatter& formatter,
                                                         const MonomialMatrix& inputMatrix) {

            // LocalityFormatView formatView{localityContext, formatter, inputMatrix.operator_matrix()()};
            if (!inputMatrix.has_operator_matrix()) {
                return export_inferred(engine, inputMatrix);
            }

            return do_export<LocalityFormatView>(engine, inputMatrix.operator_matrix()(), context, formatter);
        }

        inline matlab::data::StringArray export_locality(matlab::engine::MATLABEngine& engine,
                                                         const Locality::LocalityContext& context,
                                                         const Locality::LocalityOperatorFormatter& formatter,
                                                         const PolynomialMatrix& inputMatrix) {

            // LocalityFormatView formatView{localityContext, formatter, inputMatrix.operator_matrix()()};
            if (!inputMatrix.has_operator_matrix()) {
                [[unlikely]]
                return export_inferred(engine, inputMatrix);
            }

            return do_export<LocalityFormatView>(engine, inputMatrix.operator_matrix()(), context, formatter);
        }
    }



    SequenceStringMatrixExporter::SequenceStringMatrixExporter(matlab::engine::MATLABEngine& engine,
                                                               matlab::data::ArrayFactory& factory,
                                                               const MatrixSystem &system) noexcept
       : Exporter{engine, factory}, system{system}, localityFormatterPtr{nullptr} {
        this->localityContextPtr = nullptr; // Without formatter, do not use locality context.
        this->imsPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
    }

    SequenceStringMatrixExporter::SequenceStringMatrixExporter(matlab::engine::MATLABEngine& engine,
                                                               matlab::data::ArrayFactory& factory,
                                                               const Locality::LocalityMatrixSystem& locality_system,
                                                               const Locality::LocalityOperatorFormatter &localityFormatter) noexcept
        : Exporter{engine, factory}, system{locality_system}, localityFormatterPtr{&localityFormatter} {
            this->localityContextPtr = &locality_system.localityContext;
    }


    matlab::data::StringArray SequenceStringMatrixExporter::operator()(const MonomialMatrix &matrix) const {
        // Is this an inflation matrix? If so, display factorized format:
        if (nullptr != this->imsPtr) {
            return export_factored(engine, *this->imsPtr, matrix);
        }

        // Are we a locality system, with formatter?
        if (this->localityContextPtr != nullptr) {
            return export_locality(engine, *this->localityContextPtr, *this->localityFormatterPtr, matrix);
        }

        // Do we have direct sequences? If so, export direct (neutral) view.
        if (matrix.has_operator_matrix()) {
            const auto& op_mat = matrix.operator_matrix();
            return export_direct(engine, op_mat);
        }

        // If all else fails, use inferred string formatting
        return export_inferred(engine, matrix);
    }

    matlab::data::StringArray
    SequenceStringMatrixExporter::operator()(const PolynomialMatrix &matrix) const {
        // Are we a locality system, with formatter?
        if (this->localityContextPtr != nullptr) {
            return export_locality(engine, *this->localityContextPtr, *this->localityFormatterPtr, matrix);
        }

        // Do we have direct sequences? If so, export direct (neutral) view.
        if (matrix.has_operator_matrix()) [[unlikely]] {
             // Unlikely: Most polynomial matrices are not created from categorizing symbols in an operator matrix.
            return export_direct(engine, matrix.operator_matrix());
        }

        // If all else fails, use inferred string formatting
        return export_inferred(engine, matrix);
    }


}
