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

#include <span>

namespace Moment {
    class Context;
    class MatrixSystem;
    class PolynomialFactory;
    class ProbabilityTensor;
    class ProbabilityTensorElement;


    template<typename tensor_t> class TensorRange;
    using ProbabilityTensorRange = TensorRange<ProbabilityTensor>;

    class SymbolTable;

    namespace mex {
        class ProbabilityTensorExporter : public Exporter {
        private:
            mutable matlab::data::ArrayFactory factory;
            const Context& context;
            const SymbolTable& symbol_table;
            const PolynomialFactory& polyFactory;

        public:
            ProbabilityTensorExporter(matlab::engine::MATLABEngine& engine, const MatrixSystem& system);

            [[nodiscard]] matlab::data::CellArray sequences(const ProbabilityTensor& tensor) const;

            [[nodiscard]] matlab::data::CellArray symbols(const ProbabilityTensor& tensor) const;

            [[nodiscard]] matlab::data::CellArray sequences(const ProbabilityTensorRange& splice) const;

            [[nodiscard]] matlab::data::CellArray symbols(const ProbabilityTensorRange& splice) const;

            [[nodiscard]] matlab::data::CellArray sequence(const ProbabilityTensorElement& element) const;

            [[nodiscard]] matlab::data::CellArray symbol(const ProbabilityTensorElement& element) const;

            friend class SymbolWriterFunctor;
            friend class SequenceWriterFunctor;
        };

    }

}