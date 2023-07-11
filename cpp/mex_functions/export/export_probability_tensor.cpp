/**
 * export_probability_tensor.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_probability_tensor.h"

#include "matrix_system.h"

#include "symbolic/polynomial_factory.h"

#include "probability/probability_tensor.h"
#include "probability/virtual_polynomial_view.h"

#include "error_codes.h"

#include "export_operator_sequence.h"
#include "export_polynomial.h"

#include "utilities/reporting.h"


namespace Moment::mex {

    class SymbolCellWriterFunctor {
    private:
        const ProbabilityTensorExporter& exporter;
        PolynomialExporter polyExporter;

    public:
        explicit SymbolCellWriterFunctor(const ProbabilityTensorExporter& exporter)
            : exporter(exporter),
              polyExporter(exporter.engine, exporter.factory,
                           exporter.symbol_table, exporter.polyFactory.zero_tolerance) { }

        [[nodiscard]] matlab::data::CellArray inline operator()(const ProbabilityTensorElement& elem) const {
            // Check symbols exist
            if (!elem.hasSymbolPoly) {
                throw Moment::errors::BadPTError{"Symbols not yet found."};
            }

            return this->polyExporter.symbol_cell(elem.symbolPolynomial);
        }
    };

    class SequenceWriterFunctor {
    public:
        const bool full_export;
        const ProbabilityTensorExporter& exporter;
        const CollinsGisin& collins_gisin;
        PolynomialExporter polyExporter;

    public:
        explicit SequenceWriterFunctor(const ProbabilityTensorExporter& exporter, const bool full_export,
                                       const CollinsGisin& collins_gisin)
            : full_export{full_export}, exporter{exporter}, collins_gisin{collins_gisin},
              polyExporter(exporter.engine, exporter.factory,
                           exporter.symbol_table, exporter.polyFactory.zero_tolerance) { }

        [[nodiscard]] inline matlab::data::CellArray operator()(const ProbabilityTensorElement& elem) const {
            auto polySpec = this->fps(elem);
            return polySpec.move_to_cell(exporter.factory);
        }

        [[nodiscard]] inline FullMonomialSpecification fps(const ProbabilityTensorElement& elem) const {
            // We can do this the easy way, or the hard way...
            if (elem.hasSymbolPoly) {
                return this->polyExporter.sequences(elem.symbolPolynomial, this->full_export);
            } else {
                return this->make_from_cgpoly(elem.cgPolynomial);
            }
        }

        [[nodiscard]] FullMonomialSpecification make_from_cgpoly(const Polynomial& cgPoly) const {
            VirtualPolynomialView to_op_seq{this->collins_gisin, cgPoly};
            FullMonomialSpecification output{this->exporter.factory, to_op_seq.size(), false};
            auto write_ops = output.operators.begin();
            auto write_hash = output.hashes.begin();
            auto write_coefs = output.coefficients.begin();
            for (auto [sequence, coef] : to_op_seq) {
                *write_ops = export_operator_sequence(this->exporter.factory, sequence, true);
                *write_hash = sequence.hash();
                *write_coefs = coef;

                ++write_ops;
                ++write_hash;
                ++write_coefs;
            }
            return output;
        }
    };


    namespace {
        template<typename read_iter_t, typename export_functor_t>
        matlab::data::CellArray
        do_export(const ProbabilityTensorExporter& exporter,
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

        void remove_unused_dimensions(matlab::data::ArrayDimensions& dims) {
            // Remove elements equal to 1:
            auto last_elem = std::remove_if(dims.begin(), dims.end(), [](size_t val) {
                return val == 1;
            });
            dims.erase(last_elem, dims.end());

            // Ensure at least 2D object.
            if (dims.empty()) {
                dims.emplace_back(1);
                dims.emplace_back(1);
            } else if (dims.size() == 1) {
                dims.emplace_back(1);
            }
            assert(dims.size()>=2);
        }

    }

    ProbabilityTensorExporter::ProbabilityTensorExporter(matlab::engine::MATLABEngine &engine,
                                                         const MatrixSystem &system)
         : ExporterWithFactory{engine},
           context{system.Context()}, symbol_table{system.Symbols()}, polyFactory{system.polynomial_factory()} {

    }

    matlab::data::CellArray ProbabilityTensorExporter::sequences(const ProbabilityTensor &tensor) const {
        return do_export(*this, ProbabilityTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(), SequenceWriterFunctor{*this, false, tensor.collinsGisin});
    }

    matlab::data::CellArray ProbabilityTensorExporter::sequences(const ProbabilityTensorRange &splice) const {
        matlab::data::ArrayDimensions dims(splice.Dimensions().begin(), splice.Dimensions().end());
        remove_unused_dimensions(dims);
        return do_export(*this, std::move(dims),
                         splice.begin(), splice.end(),
                         SequenceWriterFunctor{*this, false, splice.Tensor().collinsGisin});
    }

    FullMonomialSpecification ProbabilityTensorExporter::sequence(const ProbabilityTensorElement &element,
                                                                    const CollinsGisin& cg) const {
        SequenceWriterFunctor swf{*this, false, cg};
        return swf.fps(element);
    }

    matlab::data::CellArray ProbabilityTensorExporter::sequences_with_symbols(const ProbabilityTensor &tensor) const {
        return do_export(*this, ProbabilityTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(), SequenceWriterFunctor{*this, true, tensor.collinsGisin});
    }

    matlab::data::CellArray ProbabilityTensorExporter::sequences_with_symbols(const ProbabilityTensorRange &splice) const {
        matlab::data::ArrayDimensions dims(splice.Dimensions().begin(), splice.Dimensions().end());
        remove_unused_dimensions(dims);
        return do_export(*this, std::move(dims),
                         splice.begin(), splice.end(),
                         SequenceWriterFunctor{*this, true, splice.Tensor().collinsGisin});
    }

    FullMonomialSpecification
    ProbabilityTensorExporter::sequence_with_symbols(const ProbabilityTensorElement &element,
                                                     const CollinsGisin& cg) const {
        SequenceWriterFunctor swf{*this, true, cg};
        return swf.fps(element);
    }

    matlab::data::CellArray ProbabilityTensorExporter::symbols(const ProbabilityTensor &tensor) const {

        return do_export(*this, ProbabilityTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(), SymbolCellWriterFunctor{*this});
    }

    matlab::data::CellArray ProbabilityTensorExporter::symbols(const ProbabilityTensorRange &splice) const {
        matlab::data::ArrayDimensions dims(splice.Dimensions().begin(), splice.Dimensions().end());
        remove_unused_dimensions(dims);
        return do_export(*this, std::move(dims),
                         splice.begin(), splice.end(), SymbolCellWriterFunctor{*this});
    }


    matlab::data::CellArray ProbabilityTensorExporter::symbol(const ProbabilityTensorElement &element) const {
        SymbolCellWriterFunctor swf{*this};
        return swf(element);
    }

}