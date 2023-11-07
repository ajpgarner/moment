/**
 * sequence_sign_type.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "sequence_sign_type.h"

#include <iostream>

namespace Moment {
    std::ostream& operator<<(std::ostream& os, const SequenceSignType sst) {
        switch (sst) {
            case SequenceSignType::Positive:
                os << "Positive";
                break;
            case SequenceSignType::Imaginary:
                os << "Imaginary";
                break;
            case SequenceSignType::Negative:
                os << "Negative";
                break;
            case SequenceSignType::NegativeImaginary:
                os << "NegativeImaginary";
                break;
        }
        return os;
    }
}