/**
 * site_hasher.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "site_hasher.h"
#include "site_hasher_impl.h"

namespace Moment::Pauli {

    std::unique_ptr<SiteHasher> SiteHasher::make(const size_t qubit_size, const size_t col_height) {

        size_t slides = qubit_size / SiteHasherImplBase::qubits_per_slide;
        size_t remainder = qubit_size % SiteHasherImplBase::qubits_per_slide;
        if (0 != remainder) {
            ++slides;
        }

        switch (slides) {
            case 0:
            case 1: // Specialist 1:
                return std::make_unique<SiteHasherImpl<1>>(qubit_size, col_height);
            case 2: // Specialist 2:
                return std::make_unique<SiteHasherImpl<2>>(qubit_size, col_height);
            case 3: // 'Generalist' 3:
                return std::make_unique<SiteHasherImpl<3>>(qubit_size, col_height);
            case 4: // 'Generalist' 4:
                return std::make_unique<SiteHasherImpl<4>>(qubit_size, col_height);
            case 5: // 'Generalist' 5:
                return std::make_unique<SiteHasherImpl<5>>(qubit_size, col_height);
            case 6: // 'Generalist' 6:
                return std::make_unique<SiteHasherImpl<6>>(qubit_size, col_height);
            case 7: // 'Generalist' 7:
                return std::make_unique<SiteHasherImpl<7>>(qubit_size, col_height);
            case 8: // 'Generalist' 8:
                return std::make_unique<SiteHasherImpl<8>>(qubit_size, col_height);
            default:
                throw std::runtime_error{"Could not create site hasher for this qubit size."};
        }
    }
}