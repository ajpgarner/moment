/**
 * export_polynomial_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_polynomial_tensor.h"
#include "export_operator_sequence.h"

#include "matrix_system/matrix_system.h"

#include "probability/collins_gisin.h"
#include "probability/polynomial_tensor.h"
#include "probability/virtual_polynomial_view.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"


namespace Moment::mex {
    namespace {

        template<typename read_iter_t, typename export_functor_t>
        matlab::data::CellArray
        do_export(const PolynomialTensorExporter& exporter,
                  matlab::data::ArrayDimensions&& dimensions,
                  read_iter_t read_iter, const read_iter_t read_iter_end,
                  const export_functor_t& elem_writer) {

            matlab::data::ArrayFactory& factory = exporter.factory;

            matlab::data::CellArray output = factory.createCellArray(std::move(dimensions));

            exporter.do_write(read_iter, read_iter_end,
                              output.begin(), output.end(),
                              elem_writer);

            return output;
        }
    }

    PolynomialSymbolCellWriterFunctor::PolynomialSymbolCellWriterFunctor(const PolynomialTensorExporter &exporter)
        : exporter(exporter),
          polyExporter(exporter.engine, exporter.factory, exporter.context,
                       exporter.symbol_table, exporter.polyFactory.zero_tolerance) { }

    matlab::data::CellArray
    PolynomialSymbolCellWriterFunctor::operator()(const PolynomialElement &elem) const {
        // Check symbols exist
        if (!elem.hasSymbolPoly) {
            throw Moment::errors::bad_tensor{"Symbols not yet found."};
        }

        return this->polyExporter.symbol_cell(elem.symbolPolynomial);
    }

    PolynomialSequenceWriterFunctor::PolynomialSequenceWriterFunctor(const PolynomialTensorExporter &exporter,
                                                                     const bool full_export,
                                                                     const CollinsGisin& collins_gisin)
                : full_export{full_export}, exporter{exporter}, collins_gisin{collins_gisin},
                  polyExporter(exporter.engine, exporter.factory,
                               exporter.context, exporter.symbol_table, exporter.polyFactory.zero_tolerance) { }

    matlab::data::CellArray PolynomialSequenceWriterFunctor::operator()(const PolynomialElement& elem) const {
        auto polySpec = this->fps(elem);
        return polySpec.move_to_cell(exporter.factory);
    }

    FullMonomialSpecification PolynomialSequenceWriterFunctor::fps(const PolynomialElement& elem) const {
        // We can do this the easy way, or the hard way...
        if (elem.hasSymbolPoly) {
            return this->polyExporter.sequences(elem.symbolPolynomial, this->full_export);
        } else {
            return this->make_from_cgpoly(elem.cgPolynomial);
        }
    }

    FullMonomialSpecification PolynomialSequenceWriterFunctor::make_from_cgpoly(const Polynomial &cgPoly) const {
        VirtualPolynomialView to_op_seq{this->collins_gisin, cgPoly};
        FullMonomialSpecification output{this->exporter.factory, to_op_seq.size(), false};
        auto write_ops = output.operators.begin();
        auto write_hash = output.hashes.begin();
        auto write_coefs = output.coefficients.begin();
        for (auto [sequence, coef]: to_op_seq) {
            *write_ops = export_operator_sequence(this->exporter.factory, sequence, true);
            *write_hash = sequence.hash();
            *write_coefs = coef;

            ++write_ops;
            ++write_hash;
            ++write_coefs;
        }
        return output;
    }



    PolynomialTensorExporter::PolynomialTensorExporter(matlab::engine::MATLABEngine &engine,
                                                         const MatrixSystem &system)
            : ExporterWithFactory{engine},
              context{system.Context()}, symbol_table{system.Symbols()}, polyFactory{system.polynomial_factory()} {
    }


    matlab::data::CellArray PolynomialTensorExporter::sequences(const PolynomialTensor& tensor) const {
        return do_export(*this, PolynomialTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(),
                         PolynomialSequenceWriterFunctor{*this, false, tensor.collinsGisin});
    }


    FullMonomialSpecification PolynomialTensorExporter::sequence(const PolynomialElement& element,
                                                                  const CollinsGisin& cg) const {
        PolynomialSequenceWriterFunctor swf{*this, false, cg};
        return swf.fps(element);
    }


    matlab::data::CellArray PolynomialTensorExporter::sequences_with_symbols(const PolynomialTensor &tensor) const {
        return do_export(*this, PolynomialTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(),
                         PolynomialSequenceWriterFunctor{*this, true, tensor.collinsGisin});
    }

    FullMonomialSpecification
    PolynomialTensorExporter::sequence_with_symbols(const PolynomialElement& element,
                                                     const CollinsGisin& cg) const {
        PolynomialSequenceWriterFunctor swf{*this, true, cg};
        return swf.fps(element);
    }


    matlab::data::CellArray PolynomialTensorExporter::symbols(const PolynomialTensor &tensor) const {

        return do_export(*this, PolynomialTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(), PolynomialSymbolCellWriterFunctor{*this});
    }


    matlab::data::CellArray PolynomialTensorExporter::symbol(const PolynomialElement& element) const {
        PolynomialSymbolCellWriterFunctor swf{*this};
        return swf(element);
    }



}