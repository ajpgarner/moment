/**
 * site_hasher.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_simplifier.h"
#include "nonwrapping_simplifier.h"
#include "site_hasher.h"

#include "pauli_context.h"
#include "dictionary/operator_sequence.h"

namespace Moment::Pauli {
    MomentSimplifier::MomentSimplifier(const PauliContext& context, const uint64_t label)
            : context{context}, impl_label{label}, qubits{static_cast<size_t>(context.qubit_size)},
              column_height{context.col_height > 0 ? context.col_height : qubits},
              row_width{context.col_height > 0 ? context.row_width : static_cast<size_t>(1)} {
        assert(column_height * row_width == qubits);
    }

    std::unique_ptr<MomentSimplifier> MomentSimplifier::make(const PauliContext& context) {
        // If not wrapping, we can make a simpler simplifier...
        if (context.wrap == WrapType::None) {
            if (context.is_lattice()) {
                return std::make_unique<NonwrappingLatticeSimplifier>(context);
            } else {
                return std::make_unique<NonwrappingChainSimplifier>(context);
            }
        }

        // Otherwise, test if we can support size of the wrapping simplifier...
        if (context.qubit_size > 256) {
            throw errors::bad_pauli_context{
                "Wrapping translational symmetry currently only supported for up to 256 qubits."};
        }

        // Calculate how many data slides are needed for the wrapping simplifier
        size_t slides = context.qubit_size / SiteHasherImplBase::qubits_per_slide;
        size_t remainder = context.qubit_size % SiteHasherImplBase::qubits_per_slide;
        if (0 != remainder) {
            ++slides;
        }

        // Switch and construct:
        switch (slides) {
            case 0:
            case 1: // Specialist 1:
                return std::make_unique<SiteHasher<1>>(context);
            case 2: // Specialist 2:
                return std::make_unique<SiteHasher<2>>(context);
            case 3: // 'Generalist' 3:
                return std::make_unique<SiteHasher<3>>(context);
            case 4: // 'Generalist' 4:
                return std::make_unique<SiteHasher<4>>(context);
            case 5: // 'Generalist' 5:
                return std::make_unique<SiteHasher<5>>(context);
            case 6: // 'Generalist' 6:
                return std::make_unique<SiteHasher<6>>(context);
            case 7: // 'Generalist' 7:
                return std::make_unique<SiteHasher<7>>(context);
            case 8: // 'Generalist' 8:
                return std::make_unique<SiteHasher<8>>(context);
            default:
                throw std::runtime_error{"Could not create site hasher for this qubit size."};
        }
    }

    OperatorSequence MomentSimplifier::canonical_sequence(const OperatorSequence& input) const {
        if (input.zero()) [[unlikely]] {
            return OperatorSequence::Zero(this->context);
        }
        return OperatorSequence{OperatorSequence::ConstructPresortedFlag{},
                                this->canonical_sequence(input.raw()), this->context, input.get_sign()};
    }

}