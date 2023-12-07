/**
 * site_hasher.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "site_hasher.h"
#include "site_hasher_impl.h"

#include "pauli_context.h"
#include "dictionary/operator_sequence.h"

namespace Moment::Pauli {


    SiteHasher::SiteHasher(const PauliContext& context)
            : context{context}, qubits{static_cast<size_t>(context.qubit_size)},
              column_height{context.col_height > 0 ? context.col_height : qubits},
              row_width{context.col_height > 0 ? context.row_width : static_cast<size_t>(1)} {
        assert(column_height * row_width == qubits);
    }

    std::unique_ptr<SiteHasher> SiteHasher::make(const PauliContext& context) {

        size_t slides = context.qubit_size / SiteHasherImplBase::qubits_per_slide;
        size_t remainder = context.qubit_size % SiteHasherImplBase::qubits_per_slide;
        if (0 != remainder) {
            ++slides;
        }

        switch (slides) {
            case 0:
            case 1: // Specialist 1:
                return std::make_unique<SiteHasherImpl<1>>(context);
            case 2: // Specialist 2:
                return std::make_unique<SiteHasherImpl<2>>(context);
            case 3: // 'Generalist' 3:
                return std::make_unique<SiteHasherImpl<3>>(context);
            case 4: // 'Generalist' 4:
                return std::make_unique<SiteHasherImpl<4>>(context);
            case 5: // 'Generalist' 5:
                return std::make_unique<SiteHasherImpl<5>>(context);
            case 6: // 'Generalist' 6:
                return std::make_unique<SiteHasherImpl<6>>(context);
            case 7: // 'Generalist' 7:
                return std::make_unique<SiteHasherImpl<7>>(context);
            case 8: // 'Generalist' 8:
                return std::make_unique<SiteHasherImpl<8>>(context);
            default:
                throw std::runtime_error{"Could not create site hasher for this qubit size."};
        }
    }

    OperatorSequence SiteHasher::canonical_sequence(const OperatorSequence& input) const {
        return OperatorSequence{OperatorSequence::ConstructPresortedFlag{},
                                this->canonical_sequence(input.raw()), this->context};
    }

}