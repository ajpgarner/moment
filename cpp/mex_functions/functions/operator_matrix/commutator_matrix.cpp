/**
 * extended_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "commutator_matrix.h"
#include "storage_manager.h"

#include "scenarios/pauli/pauli_matrix_system.h"

#include "import/read_localizing_matrix_indices.h"
#include "import/read_opseq_polynomial.h"
#include "import/read_polynomial.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    void CommutatorMatrixParams::extra_parse_params()  {
        assert(inputs.empty()); // Should be guaranteed by parent

        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(true);
        } else if (this->flags.contains(u"zero_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(false);
        }

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);


        // Get input type flag
        auto expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence;
        if (this->flags.contains(u"symbols")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorCell;
        }

        // Get word
        auto& word_param = this->find_or_throw(u"word");
        this->lmi_importer_ptr->read_localizing_expression(word_param, expr_type);

        // Extra params
        this->parse_optional_params();
    }

    void CommutatorMatrixParams::extra_parse_inputs() {
        // No named parameters... try to interpret inputs as [ref_id, level, word]
        assert(this->inputs.size() == 3); // should be guaranteed by parent

        // Do we offset word read by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(true);
        } else if (this->flags.contains(u"zero_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(false);
        }

        // Read NPA level
        this->lmi_importer_ptr->read_level(inputs[1]);

        // Get input type flag
        auto expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence;
        if (this->flags.contains(u"symbols")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorCell;
        }

        // Get word
        this->lmi_importer_ptr->read_localizing_expression(inputs[2], expr_type);

        // Extra params
        this->parse_optional_params();
    }

    void CommutatorMatrixParams::parse_optional_params() {
        // Get nearest-neighbour restriction if any
        this->find_and_parse(u"neighbours", [this](const matlab::data::Array& nn_param) {
            this->lmi_importer_ptr->read_nearest_neighbour(nn_param);
        });

        // Get matrix type
        if (this->flags.contains(u"anticommutator")) {
            this->requested_matrix = RequestedMatrix::Anticommutator;
        } else if (this->flags.contains(u"commutator")) {
            this->requested_matrix = RequestedMatrix::Commutator;
        }
    }

    [[nodiscard]] bool CommutatorMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool word_specified = this->params.contains(u"word");
        return level_specified || word_specified || OperatorMatrixParams::any_param_set();
    }

    CommutatorMatrixParams::CommutatorMatrixParams(SortedInputs&& input)
        : OperatorMatrixParams(std::move(input)) {
        this->lmi_importer_ptr = std::make_unique<LocalizingMatrixIndexImporter>(this->matlabEngine);

    }

    CommutatorMatrixParams::~CommutatorMatrixParams() noexcept = default;



    CommutatorMatrix::CommutatorMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level, extensions] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");

        // Word type?
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"operators");
        this->mutex_params.add_mutex(u"symbols", u"operators");

        // Matrix type?
        this->flag_names.emplace(u"anticommutator");
        this->flag_names.emplace(u"commutator");
        this->mutex_params.add_mutex(u"anticommutator", u"commutator");

        this->min_inputs = 0;
        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    getMonoCM(matlab::engine::MATLABEngine& matlabEngine, Pauli::PauliMatrixSystem& pauli_system,
              CommutatorMatrixParams& input, Multithreading::MultiThreadPolicy mt_policy) {
        auto read_lock = pauli_system.get_read_lock();
        auto plmi = input.lmi_importer().to_pauli_monomial_index();
        auto offset = pauli_system.CommutatorMatrices.find_index(plmi);
        if (offset >= 0) {
            return {offset, pauli_system[offset]};
        }

        // Try to create
        read_lock.unlock();
        return pauli_system.CommutatorMatrices.create(plmi, mt_policy);
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    getMonoACM(matlab::engine::MATLABEngine& matlabEngine, Pauli::PauliMatrixSystem& pauli_system,
              CommutatorMatrixParams& input, Multithreading::MultiThreadPolicy mt_policy) {
        auto read_lock = pauli_system.get_read_lock();
        auto plmi = input.lmi_importer().to_pauli_monomial_index();
        auto offset = pauli_system.AnticommutatorMatrices.find_index(plmi);
        if (offset >= 0) {
            return {offset, pauli_system[offset]};
        }

        // Try to create
        read_lock.unlock();
        return pauli_system.AnticommutatorMatrices.create(plmi, mt_policy);
    }

    std::pair<size_t, const Moment::SymbolicMatrix&>
    CommutatorMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp)  {
        // Get extended parameters
        auto& cmp = dynamic_cast<CommutatorMatrixParams&>(omp);

        // Get MT policy
        const auto mt_policy = this->settings->get_mt_policy();

        // Get Pauli matrix system
        auto * pauli_ptr = dynamic_cast<Pauli::PauliMatrixSystem*>(&system);
        if (nullptr == pauli_ptr) {
            throw_error(this->matlabEngine, errors::bad_param, "Matrix system reference was not a Pauli scenario.");
        }
        auto& pauli_system = *pauli_ptr;

        // Attach matrix system to index reader
        cmp.lmi_importer().link_matrix_system(&pauli_system);

        const bool anticommutator = cmp.requested_matrix == CommutatorMatrixParams::RequestedMatrix::Anticommutator;

        // Switch based on type
        try {
            switch (cmp.lmi_importer().get_expression_type()) {
                case LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence:
                    if (anticommutator) {
                        return getMonoACM(matlabEngine, pauli_system, cmp, this->settings->get_mt_policy());
                    } else {
                        return getMonoCM(matlabEngine, pauli_system, cmp, this->settings->get_mt_policy());
                    }
                case LocalizingMatrixIndexImporter::ExpressionType::SymbolCell:
                    throw_error(matlabEngine, errors::internal_error,
                                "Currently symbol-cell inputs for (anti)commutator matrices are not supported.");
                case LocalizingMatrixIndexImporter::ExpressionType::OperatorCell:
                    throw_error(matlabEngine, errors::internal_error,
                                "Currently operator cell inputs for (anti)commutator matrices are not supported.");
                    //return getPolyOpCM(matlabEngine, system, cmp, this->settings->get_mt_policy());
                default:
                case LocalizingMatrixIndexImporter::ExpressionType::Unknown:
                    throw_error(matlabEngine, errors::internal_error, "Unknown localizing expression type.");
            }
        } catch (std::exception& e) {
            throw_error(matlabEngine, errors::internal_error,
                        std::string("A problem occurred while retrieving/generating localizing matrix: ") + e.what());
        }
    }

}