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

#include "error_codes.h"
#include "export_polynomial.h"
#include "utilities/reporting.h"



namespace Moment::mex {

    class SymbolWriterFunctor {
    private:
        const ProbabilityTensorExporter& exporter;
        PolynomialExporter polyExporter;

    public:
        explicit SymbolWriterFunctor(const ProbabilityTensorExporter& exporter)
            : exporter(exporter),
              polyExporter(exporter.engine, exporter.symbol_table, exporter.polyFactory.zero_tolerance) { }

        [[nodiscard]] matlab::data::CellArray operator()(const ProbabilityTensorElement& elem) const {
            // Check symbols exist
            if (!elem.hasSymbolPoly) {
                throw Moment::errors::BadPTError{"Symbols not yet found."};
            }

            return this->polyExporter.direct(elem.symbolPolynomial);
        }
    };

    class SequenceWriterFunctor {
    private:
        const ProbabilityTensorExporter& exporter;
        PolynomialExporter polyExporter;

    public:
        explicit SequenceWriterFunctor(const ProbabilityTensorExporter& exporter)
            : exporter(exporter),
              polyExporter(exporter.engine, exporter.symbol_table, exporter.polyFactory.zero_tolerance) { }

        [[nodiscard]] matlab::data::CellArray operator()(const ProbabilityTensorElement& elem) const {

            // We can do this the easy way, or the hard way...
            if (elem.hasSymbolPoly) {
                return this->polyExporter.sequences(elem.symbolPolynomial);
            } else {
                throw Moment::errors::BadPTError{"Symbol deduction not yet implemented."};
            }
        }
    };


    namespace {

        template<typename read_iter_t, typename write_elem_functor_t>
        matlab::data::CellArray
        do_export(matlab::engine::MATLABEngine& engine,
                  matlab::data::ArrayFactory& factory,
                  matlab::data::ArrayDimensions&& dimensions,
                  read_iter_t read_iter, const read_iter_t read_iter_end,
                  const write_elem_functor_t& elem_writer) {

            matlab::data::CellArray output = factory.createCellArray(std::move(dimensions));

            auto write_iter = output.begin();

            while ((read_iter != read_iter_end) && (write_iter != output.end())) {
                *write_iter = elem_writer(*read_iter);
                ++read_iter;
                ++write_iter;
            }

            // Sanity checks
            if (read_iter != read_iter_end) {
                throw_error(engine, errors::internal_error,
                            "Tensor write exceeds expected dimensions.");
            }
            if (write_iter != output.end()) {
                throw_error(engine, errors::internal_error,
                            "Unexpected encountered end of tensor before write was complete.");
            }

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
         : Exporter{engine}, factory{},
           context{system.Context()}, symbol_table{system.Symbols()}, polyFactory{system.polynomial_factory()} {

    }

    matlab::data::CellArray ProbabilityTensorExporter::sequences(const ProbabilityTensor &tensor) const {
        return do_export(this->engine, this->factory,
                         ProbabilityTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(),
                         SequenceWriterFunctor{*this});
    }

    matlab::data::CellArray ProbabilityTensorExporter::sequences(const ProbabilityTensorRange &splice) const {
        auto dims = splice.Dimensions();
        remove_unused_dimensions(dims);
        return do_export(this->engine, this->factory,
                         std::move(dims),
                         splice.begin(), splice.end(),
                         SequenceWriterFunctor{*this});
    }

    matlab::data::CellArray ProbabilityTensorExporter::symbols(const ProbabilityTensor &tensor) const {

        return do_export(this->engine, this->factory,
                         ProbabilityTensor::Index{tensor.Dimensions},
                         tensor.begin(), tensor.end(),
                         SymbolWriterFunctor{*this});
    }

    matlab::data::CellArray ProbabilityTensorExporter::symbols(const ProbabilityTensorRange &splice) const {
        auto dims = splice.Dimensions();
        remove_unused_dimensions(dims);
        return do_export(this->engine, this->factory,
                         std::move(dims),
                         splice.begin(), splice.end(),
                         SymbolWriterFunctor{*this});
    }

    matlab::data::CellArray ProbabilityTensorExporter::sequence(const ProbabilityTensorElement &element) const {
        SequenceWriterFunctor swf{*this};
        return swf(element);
    }

    matlab::data::CellArray ProbabilityTensorExporter::symbol(const ProbabilityTensorElement &element) const {
        SymbolWriterFunctor swf{*this};
        return swf(element);
    }

}