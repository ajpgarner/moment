/**
 * export_polynomial_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "MatlabDataArray.hpp"

#include "exporter.h"
#include "export_polynomial.h"
#include "full_monomial_specification.h"

namespace Moment {
    class Context;
    class CollinsGisin;
    class MatrixSystem;
    class SymbolTable;
    class PolynomialFactory;
    class PolynomialTensor;
    struct PolynomialElement;

    namespace mex {

        class PolynomialTensorExporter;

        class PolynomialSymbolCellWriterFunctor {
        private:
            const PolynomialTensorExporter& exporter;
            PolynomialExporter polyExporter;

        public:
            explicit PolynomialSymbolCellWriterFunctor(const PolynomialTensorExporter& exporter);

            [[nodiscard]] matlab::data::CellArray operator()(const PolynomialElement& elem) const;
        };

        class PolynomialSequenceWriterFunctor {
        public:
            const bool full_export;
            const PolynomialTensorExporter &exporter;
            const CollinsGisin &collins_gisin;
            PolynomialExporter polyExporter;

        public:
            explicit PolynomialSequenceWriterFunctor(const PolynomialTensorExporter& exporter, const bool full_export,
                                                     const CollinsGisin &collins_gisin);

            [[nodiscard]] matlab::data::CellArray operator()(const PolynomialElement& elem) const;

            [[nodiscard]] FullMonomialSpecification fps(const PolynomialElement& elem) const;

            [[nodiscard]] FullMonomialSpecification make_from_cgpoly(const Polynomial &cgPoly) const;
        };

        class PolynomialTensorExporter : public ExporterWithFactory {
        public:
            const Context& context;
            const SymbolTable& symbol_table;
            const PolynomialFactory& polyFactory;

        public:
            PolynomialTensorExporter(matlab::engine::MATLABEngine& engine, const MatrixSystem& system);

            /**
             * Write a single element as a polynomial.
             */
            [[nodiscard]] FullMonomialSpecification sequence(const PolynomialElement& element,
                                                             const CollinsGisin& cg) const;


            /**
             * Write the entire tensor as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences(const PolynomialTensor& tensor) const;


            /**
             * Write a single element as a polynomial.
             */
            [[nodiscard]] FullMonomialSpecification sequence_with_symbols(const PolynomialElement& element,
                                                                          const CollinsGisin& cg) const;


            /**
             * Write the entire tensor as a polynomial.
             */
            [[nodiscard]] matlab::data::CellArray sequences_with_symbols(const PolynomialTensor& tensor) const;


            /**
             * Write a single element as a symbol cell.
             */
            [[nodiscard]] matlab::data::CellArray symbol(const PolynomialElement& element) const;

            /**
             * Write the entire tensor as a symbol cell.
             */
            [[nodiscard]] matlab::data::CellArray symbols(const PolynomialTensor& tensor) const;

            friend class SymbolCellWriterFunctor;
            friend class SequenceWriterFunctor;
        };
    }
}