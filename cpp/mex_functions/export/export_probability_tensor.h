/**
 * export_probability_tensor.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "export_polynomial_tensor.h"

#include <span>

namespace Moment {

    class ProbabilityTensor;

    template<typename tensor_t> class TensorRange;
    using ProbabilityTensorRange = TensorRange<ProbabilityTensor>;

    class SymbolTable;

    namespace mex {
        class ProbabilityTensorExporter : public PolynomialTensorExporter {
        public:
            ProbabilityTensorExporter(matlab::engine::MATLABEngine& engine, const MatrixSystem& system)
                : PolynomialTensorExporter{engine, system} { }

            using PolynomialTensorExporter::sequences;
            using PolynomialTensorExporter::sequences_with_symbols;
            using PolynomialTensorExporter::symbols;

            /**
             * Write the tensor slice as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences(const ProbabilityTensorRange& splice) const;

            /**
             * Write the tensor slice as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences_with_symbols(const ProbabilityTensorRange& splice) const;

            /**
             * Write the tensor slice as a symbol cell
             */
            [[nodiscard]] matlab::data::CellArray symbols(const ProbabilityTensorRange& splice) const;


            friend class SymbolCellWriterFunctor;
            friend class SequenceWriterFunctor;
        };

    }

}