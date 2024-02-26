/**
 * lattice_symmetrize.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "lattice_symmetrize.h"

#include "storage_manager.h"

#include "dictionary/operator_sequence.h"
#include "dictionary/raw_polynomial.h"

#include "scenarios/contextual_os.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/symmetry/lattice_duplicator.h"

#include "export/export_polynomial.h"

#include "utilities/read_as_scalar.h"

#include <cassert>

namespace Moment::mex::functions {

    LatticeSymmetrizeParams::LatticeSymmetrizeParams(Moment::mex::SortedInputs &&rawInputs)
            : SortedInputs{std::move(rawInputs)}, matrix_system_key{matlabEngine} {
        this->matrix_system_key.parse_input(this->inputs[0]);

        if (this->inputs[1].isEmpty() || (this->inputs[1].getType() != matlab::data::ArrayType::CELL)) {
            throw BadParameter{"Argument must be an operator cell."};
        }
        const matlab::data::CellArray as_cell = inputs[1];
        assert(!as_cell.isEmpty());
        this->input_polynomial = std::make_unique<StagingPolynomial>(matlabEngine, *as_cell.begin(), "Polynomial");
    }

    LatticeSymmetrize::LatticeSymmetrize(matlab::engine::MATLABEngine &matlabEngine, Moment::mex::StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void LatticeSymmetrize::operator()(IOArgumentRange output, LatticeSymmetrizeParams& input) {
        // Attempt to get matrix system, and cast to ImportedMatrixSystem:
        std::shared_ptr<MatrixSystem> matrixSystemPtr = input.matrix_system_key(this->storageManager);

        // Attempt to cast to Pauli matrix system:
        auto* pmsPtr = dynamic_cast<Pauli::PauliMatrixSystem*>(matrixSystemPtr.get());
        if (nullptr == pmsPtr) {
            std::stringstream errSS;
            errSS << "`lattice_symmetrize` can only be called for objects in the Pauli scenario:\n"
                  << "MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec
                  << " was not a valid PauliMatrixSystem.";
            throw BadParameter{errSS.str()};
        }
        Pauli::PauliMatrixSystem& pms = *pmsPtr;

        // Check if PMS has any symmetry
        const auto& context = pms.pauliContext;
        if (context.translational_symmetry != Pauli::SymmetryType::Translational) {
            throw BadParameter{"This Pauli scenario has no translational symmetry."};
        }

        // Stage polynomial input, and convert to operator sequences
        input.input_polynomial->supply_context(context);
        const auto raw_input_poly = input.input_polynomial->to_raw_polynomial();

        // Print out input:
        if (this->debug) {
            std::stringstream inputSS;
            ContextualOS cSS{inputSS, pms.Context(), pms.Symbols()};
            for (auto& [os, factor] : raw_input_poly) {
                cSS << os << " * " << factor << "\n";
            }
            print_to_console(this->matlabEngine, inputSS.str());
        }

        // Do symmetrization:
        const RawPolynomial raw_output_poly = Pauli::LatticeDuplicator::symmetrical_copy(context, raw_input_poly);

        // Export as polynomial specification
        matlab::data::ArrayFactory factory;
        PolynomialExporter pe{this->matlabEngine, factory,
                              context, pms.Symbols(), pms.polynomial_factory().zero_tolerance};
        auto fms = pe.sequences(raw_output_poly);
        output[0] = fms.move_to_cell(factory);
    }
}