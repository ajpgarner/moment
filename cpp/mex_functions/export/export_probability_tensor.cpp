/**
 * export_probability_tensor.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_probability_tensor.h"

#include "matrix_system/matrix_system.h"

#include "symbolic/polynomial_factory.h"

#include "probability/probability_tensor.h"
#include "probability/virtual_polynomial_view.h"

#include "error_codes.h"

#include "export_operator_sequence.h"
#include "export_polynomial.h"

#include "utilities/reporting.h"


namespace Moment::mex {

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


    matlab::data::CellArray ProbabilityTensorExporter::sequences(const ProbabilityTensorRange &splice) const {
        matlab::data::ArrayDimensions dims(splice.Dimensions().begin(), splice.Dimensions().end());
        remove_unused_dimensions(dims);
        return do_export(*this, std::move(dims),
                         splice.begin(), splice.end(),
                         PolynomialSequenceWriterFunctor{*this, false, splice.Tensor().collinsGisin});
    }



    matlab::data::CellArray ProbabilityTensorExporter::sequences_with_symbols(const ProbabilityTensorRange &splice) const {
        matlab::data::ArrayDimensions dims(splice.Dimensions().begin(), splice.Dimensions().end());
        remove_unused_dimensions(dims);
        return do_export(*this, std::move(dims),
                         splice.begin(), splice.end(),
                         PolynomialSequenceWriterFunctor{*this, true, splice.Tensor().collinsGisin});
    }

    matlab::data::CellArray ProbabilityTensorExporter::symbols(const ProbabilityTensorRange &splice) const {
        matlab::data::ArrayDimensions dims(splice.Dimensions().begin(), splice.Dimensions().end());
        remove_unused_dimensions(dims);
        return do_export(*this, std::move(dims),
                         splice.begin(), splice.end(), PolynomialSymbolCellWriterFunctor{*this});
    }
}