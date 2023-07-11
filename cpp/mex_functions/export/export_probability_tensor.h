/**
 * export_probability_tensor.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "exporter.h"
#include "export_polynomial.h"

#include <span>

namespace Moment {
    class Context;
    class CollinsGisin;
    class MatrixSystem;
    class PolynomialFactory;
    class ProbabilityTensor;
    class ProbabilityTensorElement;


    template<typename tensor_t> class TensorRange;
    using ProbabilityTensorRange = TensorRange<ProbabilityTensor>;

    class SymbolTable;

    namespace mex {
        class ProbabilityTensorExporter : public ExporterWithFactory {
        public:
            const Context& context;
            const SymbolTable& symbol_table;
            const PolynomialFactory& polyFactory;

        public:
            ProbabilityTensorExporter(matlab::engine::MATLABEngine& engine, const MatrixSystem& system);

            /**
             * Write the entire tensor as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences(const ProbabilityTensor& tensor) const;

            /**
             * Write the tensor slice as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences(const ProbabilityTensorRange& splice) const;

            /**
             * Write a single element as a polynomial.
             */
            [[nodiscard]] FullMonomialSpecification sequence(const ProbabilityTensorElement& element,
                                                               const CollinsGisin& cg) const;

            /**
             * Write the entire tensor as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences_with_symbols(const ProbabilityTensor& tensor) const;

            /**
             * Write the tensor slice as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences_with_symbols(const ProbabilityTensorRange& splice) const;

            /**
             * Write a single element as a polynomial.
             */
            [[nodiscard]] FullMonomialSpecification sequence_with_symbols(const ProbabilityTensorElement& element,
                                                                            const CollinsGisin& cg) const;

            /**
             * Write the entire tensor as a symbol cell.
             */
            [[nodiscard]] matlab::data::CellArray symbols(const ProbabilityTensor& tensor) const;

            /**
             * Write the tensor slice as a symbol cell
             */
            [[nodiscard]] matlab::data::CellArray symbols(const ProbabilityTensorRange& splice) const;

            /**
             * Write a single element as a symbol cell.
             */
            [[nodiscard]] matlab::data::CellArray symbol(const ProbabilityTensorElement& element) const;

            friend class SymbolCellWriterFunctor;
            friend class SequenceWriterFunctor;
        };

    }

}