/**
 * export_osg.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_osg.h"

#include "dictionary/operator_sequence_generator.h"

#include "export_operator_sequence.h"
#include "full_monomial_specification.h"

#include "utilities/reporting.h"

namespace Moment::mex {
    namespace {

    }

    matlab::data::CellArray OSGExporter::operators(const OperatorSequenceGenerator &osg,
                                                   const bool matlab_indexing) const {
        auto output = this->factory.createCellArray(matlab::data::ArrayDimensions{osg.size(), 1});

        auto write_iter = output.begin();
        for (const auto& seq : osg) {
            *write_iter = export_operator_sequence(this->factory, seq, matlab_indexing);
            ++write_iter;
        }
        assert(write_iter == output.end());

        return output;
    }

    void OSGExporter::sequences(IOArgumentRange &output, const OperatorSequenceGenerator &osg) const {
        const size_t element_count = osg.size();
        FullMonomialSpecification monomial{factory, element_count, false};

        this->do_write(osg.begin(), osg.end(),
                       monomial.partial_write_begin(), monomial.partial_write_end(),
                       FullMonomialSpecification::PartialWriteFunctor{this->factory, this->symbols});

        monomial.move_to_output(output);
    }

    void OSGExporter::sequences_with_symbol_info(IOArgumentRange &output, const OperatorSequenceGenerator &osg) const {
        const size_t element_count = osg.size();
        FullMonomialSpecification monomial{factory, element_count, true};

        try {
            this->do_write(osg.begin(), osg.end(),
                           monomial.full_write_begin(), monomial.full_write_end(),
                           FullMonomialSpecification::FullWriteFunctor{this->factory, this->symbols});

        } catch (const FullMonomialSpecification::missing_symbol_error& mse) {
            throw InternalError{std::string("Cannot export word list: ") + mse.what()};
        }
        monomial.move_to_output(output);
    }
}