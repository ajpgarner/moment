/**
 * moment_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"
#include "operator_matrix_factory.h"

#include "dictionary/dictionary.h"
#include "dictionary/operator_sequence_generator.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include "scenarios/context.h"

#include <limits>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment {
    std::string MomentMatrix::description() const {
        std::stringstream ss;
        ss << "Moment Matrix, Level " << this->Index;
        return ss.str();
    }
}