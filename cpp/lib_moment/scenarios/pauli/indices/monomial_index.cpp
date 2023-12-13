/**
 * monomial_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "monomial_index.h"

#include <sstream>

namespace Moment::Pauli {
    namespace {
        [[nodiscard]] inline std::string do_make_name(const std::string& matrix_type_name,
                                                      const NearestNeighbourIndex& nn_info,
                                                      const OperatorSequence& word) {

            std::stringstream ss;
            ss << matrix_type_name << " Matrix, Level " << nn_info.moment_matrix_level;
            if (nn_info.neighbours > 0) {
                ss << ", " << nn_info.neighbours << " Nearest Neighbour";
                if (nn_info.neighbours != 1) {
                    ss << "s";
                }
            }
            ss << ", Word " << word;
            return ss.str();
        }
    }

    std::string LocalizingMatrixIndex::to_string() const {
        return do_make_name("Localizing", this->Index, this->Word);
    }

    std::string CommutatorMatrixIndex::to_string() const {
        return do_make_name("Commutator", this->Index, this->Word);
    }

    std::string AnticommutatorMatrixIndex::to_string() const {
        return do_make_name("Anticommutator", this->Index, this->Word);
    }
}