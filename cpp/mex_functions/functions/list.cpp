/**
 * list.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "list.h"

#include "matrix_system.h"

#include "matrix/symbolic_matrix.h"
#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "storage_manager.h"

#include <cassert>

namespace Moment::mex::functions {
    namespace {
        void output_ms_info(std::ostream& os, uint32_t id, const MatrixSystem& ms) {
            auto read_lock = ms.get_read_lock();
            const auto& symbols = ms.Symbols();

            os << "System #" << id << ": " << ms.system_type_name();
            const size_t symbol_count = ms.Symbols().size();
            os << ": " << symbol_count << " " << (symbol_count != 1 ? "symbols" : "symbol") << ", ";
            const size_t matrix_count = ms.size();
            os << matrix_count << " " << (matrix_count != 1 ? "matrices" : "matrix");

            for (size_t matrix_index = 0; matrix_index < matrix_count; ++matrix_index) {
                const auto& matrix = ms[matrix_index];
                os << "\n " << matrix_index << ": " << matrix.Dimension() << "x" << matrix.Dimension();
                os << " " << matrix.description();
            }
        }
    }

    ListParams::ListParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputs)
        : SortedInputs(std::move(inputs)) {
        if (this->inputs.size() >= 1) {
            this->output_type = OutputType::OneSystem;
            this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);
        } else {
            this->output_type = OutputType::All;
        }
    }

    List::List(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : Moment::mex::functions::MexFunction(matlabEngine, storage, MEXEntryPointID::List, u"list") {
        this->min_inputs = 0;
        this->max_inputs = 1;
        this->min_outputs = 0;
        this->max_outputs = 1;
    }

    std::unique_ptr<SortedInputs> List::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        auto txInput = std::make_unique<ListParams>(this->matlabEngine, std::move(*input));
        if ((txInput->output_type == ListParams::OutputType::OneSystem)
            && (!this->storageManager.MatrixSystems.check_signature(txInput->matrix_system_key))) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
        return txInput;
    }

    void List::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputRaw) {
        assert(inputRaw);
        auto& input = dynamic_cast<ListParams&>(*inputRaw);

        bool output_to_console = (output.size() == 0);

        std::stringstream ss;
        if (input.output_type == ListParams::OutputType::All) {
            auto [id, msPtr] = this->storageManager.MatrixSystems.first();
            bool done_one = false;
            if ((id != 0xffffffff) && (msPtr != nullptr)) {
                do {
                    if (done_one) {
                        ss << "\n";
                    } else {
                        done_one = true;
                    }
                    output_ms_info(ss, id, *msPtr);
                    // Get next:
                    auto [next_id, nextPtr] = this->storageManager.MatrixSystems.next(id);
                    id = next_id;
                    msPtr = nextPtr;
                } while ((id != 0xffffffff) && (msPtr != nullptr));
            } else {
                ss << "No matrix systems defined.";
            }
        } else {
            auto id = PersistentStorage<MatrixSystem>::get_index(input.matrix_system_key);
            auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
            output_ms_info(ss, id, *msPtr);
        }


        if (output_to_console) {
            ss << "\n";
            print_to_console(this->matlabEngine, ss.str());
        } else {
            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar(ss.str());
        }
    }
}