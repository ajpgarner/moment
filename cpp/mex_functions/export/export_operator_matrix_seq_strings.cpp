/**
 * export_operator_matrix_seq_strings.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 */

#include "export_operator_matrix_seq_strings.h"

#include "matrix_system/matrix_system.h"
#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "symbolic/symbol_table.h"
#include "scenarios/context.h"
#include "scenarios/contextual_os_helper.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/reporting.h"

#include "utilities/format_factor.h"
#include "utilities/utf_conversion.h"

#include "errors.h"

#include "mex.hpp"

namespace Moment::mex {

    namespace {

        template<typename matrix_elem_t>
        class FormatView {
        public:
            using raw_const_iterator = typename SquareMatrix<matrix_elem_t>::const_iterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const StringFormatContext * sfc = nullptr;
                raw_const_iterator raw_iter;
                UTF8toUTF16Convertor convertor;

            public:
                constexpr const_iterator(const StringFormatContext * sfc, raw_const_iterator rci)
                        :  sfc{sfc}, raw_iter{rci} {
                    assert(sfc);
                }

                constexpr bool operator==(const const_iterator& rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator& rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) &{
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(sfc != nullptr);
                    return {convertor(make_contextualized_string(*sfc, [this](ContextualOS& os){
                        os << *raw_iter;
                    }))};
                }
            };

            static_assert(std::input_iterator<FormatView<matrix_elem_t>::const_iterator>);

        private:
            const StringFormatContext sfc;
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            FormatView(StringFormatContext the_sfc, const SquareMatrix<matrix_elem_t>& inputMatrix)
                    : sfc{the_sfc},
                      iter_begin{&sfc, inputMatrix.begin()},
                      iter_end{&sfc, inputMatrix.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }

            [[nodiscard]] auto end() const { return iter_end; }

        };

        using DirectFormatView = FormatView<OperatorSequence>;
        using InferredFormatView = FormatView<Monomial>;
        using InferredPolynomialFormatView = FormatView<Polynomial>;

        class FactorFormatView {
        public:
            using raw_const_iterator = SquareMatrix<Monomial>::const_iterator;

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
                constexpr const_iterator(const Inflation::InflationContext& context,
                                         const Inflation::FactorTable& factors,
                                         raw_const_iterator rci)
                        : context{&context}, factors{&factors}, raw_iter{rci} {}

                constexpr bool operator==(const const_iterator &rhs) const noexcept {
                    return this->raw_iter == rhs.raw_iter;
                }

                constexpr bool operator!=(const const_iterator &rhs) const noexcept {
                    return this->raw_iter != rhs.raw_iter;
                }

                constexpr const_iterator& operator++() {
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
                    : iter_begin{context, factors, inputMatrix.begin()},
                      iter_end{context, factors, inputMatrix.end()} {
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
                throw InternalError{"export_symbol_matrix index count mismatch: too few input elements."};
            }
            if (readIter != formatView.end()) {
                throw InternalError{"export_symbol_matrix index count mismatch: too many input elements."};
            }

            return outputArray;
        }

        inline matlab::data::StringArray export_direct(matlab::engine::MATLABEngine& engine,
                                                       const Context& context,
                                                       const SymbolTable& symbols,
                                                       const OperatorMatrix &opMatrix)  {
            StringFormatContext sfc{context, symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

            return do_export<DirectFormatView>(engine, opMatrix(), sfc);
        }

        inline matlab::data::StringArray export_only_symbols(matlab::engine::MATLABEngine& engine,
                                                       const Context& context,
                                                       const SymbolTable& symbols,
                                                       const MonomialMatrix& inputMatrix)  {
            StringFormatContext sfc{context, symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.prefactor_join = StringFormatContext::PrefactorJoin::Nothing;
            sfc.format_info.hash_before_symbol_id = true;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::SymbolIds;

            return do_export<InferredFormatView>(engine, inputMatrix.SymbolMatrix(), sfc);
        }

        inline matlab::data::StringArray export_inferred(matlab::engine::MATLABEngine& engine,
                                                         const Context& context,
                                                         const SymbolTable& symbols,
                                                         const MonomialMatrix &inputMatrix) {
            StringFormatContext sfc{context, symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

            return do_export<InferredFormatView>(engine, inputMatrix.SymbolMatrix(), sfc);
        }

        inline matlab::data::StringArray export_inferred(matlab::engine::MATLABEngine& engine,
                                                         const Context& context,
                                                         const SymbolTable& symbols,
                                                         const PolynomialMatrix &inputMatrix) {
            StringFormatContext sfc{context, symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

            return do_export<InferredPolynomialFormatView>(engine, inputMatrix.SymbolMatrix(), sfc);
        }

        inline matlab::data::StringArray export_factored(matlab::engine::MATLABEngine& engine,
                                                         const Inflation::InflationMatrixSystem& ims,
                                                         const MonomialMatrix &inputMatrix)  {

            return do_export<FactorFormatView>(engine, inputMatrix.SymbolMatrix(),
                                               ims.InflationContext(), ims.Factors());
        }

        template<typename matrix_t>
        inline matlab::data::StringArray export_locality(matlab::engine::MATLABEngine& engine,
                                                         const Locality::LocalityContext& context,
                                                         const SymbolTable& symbols,
                                                         const Locality::LocalityOperatorFormatter& formatter,
                                                         const matrix_t& inputMatrix) {
            // Set up output context
            StringFormatContext sfc{context, symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.locality_formatter = &formatter;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

            // Export
            if (!inputMatrix.has_aliased_operator_matrix()) {
                using AppropriateInferredFormatView = FormatView<typename matrix_t::ElementType>;
                return do_export<AppropriateInferredFormatView>(engine, inputMatrix.SymbolMatrix(), sfc);
            } else {
                return do_export<DirectFormatView>(engine, inputMatrix.aliased_operator_matrix()(), sfc);
            }
        }
    }



    SequenceStringMatrixExporter::SequenceStringMatrixExporter(matlab::engine::MATLABEngine& engine,
                                                               matlab::data::ArrayFactory& factory,
                                                               const MatrixSystem &system) noexcept
       : Exporter{engine, factory}, system{system} {
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
        // Does system even define operators?
        if (!this->system.Context().defines_operators()) {
            return export_only_symbols(engine, this->system.Context(), this->system.Symbols(), matrix);
        }

        // Is this an inflation matrix? If so, display factorized format:
        if (nullptr != this->imsPtr) {
            return export_factored(engine, *this->imsPtr, matrix);
        }

        // Are we a locality system, with formatter?
        if (this->localityContextPtr != nullptr) {
            return export_locality(engine, *this->localityContextPtr, this->system.Symbols(),
                                   *this->localityFormatterPtr, matrix);
        }

        // Do we have direct sequences? If so, export direct (neutral) view.
        if (matrix.has_aliased_operator_matrix()) {
            const auto& op_mat = matrix.aliased_operator_matrix();
            return export_direct(engine, this->system.Context(), this->system.Symbols(), op_mat);
        }

        // If all else fails, use inferred string formatting
        return export_inferred(engine, this->system.Context(), this->system.Symbols(), matrix);
    }

    matlab::data::StringArray
    SequenceStringMatrixExporter::operator()(const PolynomialMatrix &matrix) const {
        // Are we a locality system, with formatter?
        if (this->localityContextPtr != nullptr) {
            return export_locality(engine, *this->localityContextPtr, this->system.Symbols(),
                                   *this->localityFormatterPtr, matrix);
        }

        // Do we have direct sequences? If so, export direct (neutral) view.
        if (matrix.has_aliased_operator_matrix()) [[unlikely]] {
             // Unlikely: Most polynomial matrices are not created from categorizing symbols in an operator matrix.
            return export_direct(engine, this->system.Context(), this->system.Symbols(), matrix.aliased_operator_matrix());
        }

        // If all else fails, use inferred string formatting
        return export_inferred(engine, this->system.Context(), this->system.Symbols(), matrix);
    }


}
