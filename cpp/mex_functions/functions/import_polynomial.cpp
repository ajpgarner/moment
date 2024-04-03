/**
 * import_polynomial.cpp
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "import_polynomial.h"

#include "utilities/read_choice.h"
#include "export/export_polynomial.h"

#include "scenarios/context.h"
#include "matrix_system/matrix_system.h"

#include <atomic>

namespace Moment::mex::functions {
    ImportPolynomialParams::ImportPolynomialParams(SortedInputs&& rawInputs)
        : SortedInputs{std::move(rawInputs)}, matrix_system_key{matlabEngine} {

        // Read matrix system
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Attempt to read polynomial
        if (inputs[1].getType() != matlab::data::ArrayType::CELL) {
            throw BadParameter{"Polynomial simplify expects cell input."};
        }

        // Copy input dimensions
        const auto input_dims = inputs[1].getDimensions();
        this->input_shape.reserve(input_dims.size());
        std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));
        this->inputPolynomials.reserve(inputs[1].getNumberOfElements());

        // Looks suspicious, but promised by MATLAB to be a reference, not copy.
        const matlab::data::CellArray cell_input = inputs[1];
        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            this->inputPolynomials.emplace_back(read_raw_polynomial_data(this->matlabEngine, "Input", *read_iter));
            ++read_iter;
        }

        // Flag if we should register unknown symbols
        this->register_new = this->flags.contains(u"register");

        // Choose output type
        this->find_and_parse(u"output", [this](const matlab::data::Array& outputModeArray) {
            switch (read_choice("output", {"string", "symbol"}, outputModeArray)) {
                case 0:
                    this->output_type = OutputType::String;
                    break;
                case 1:
                    this->output_type = OutputType::SymbolCell;
                    break;
            }
        });
    }

    ImportPolynomial::ImportPolynomial(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction(matlabEngine, storage) {
        this->min_inputs = this->max_inputs = 2;
        this->min_outputs = this->max_outputs = 1;
        this->flag_names.insert(u"register");

        this->param_names.insert(u"output");
    }

    void ImportPolynomial::operator()(IOArgumentRange output, ImportPolynomialParams& input) {

        // Get matrix system
        auto msPtr = input.matrix_system_key(this->storageManager);
        auto& matrix_system = *msPtr;
        auto lock = matrix_system.get_read_lock();

        const auto& poly_factory = matrix_system.polynomial_factory();

        // Checks, if in registration mode:
        if (input.register_new) {
            // Check largest symbol against symbol table
            uint64_t largest_symbol = 1;
            for (const auto& input_poly: input.inputPolynomials) {
                for (const auto& mono : input_poly) {
                    largest_symbol = std::max(largest_symbol, mono.symbol_id);
                }
            }

            const bool could_need_new_symbols = matrix_system.Symbols().size() < (largest_symbol+1);

            if (could_need_new_symbols) {
                // Swap from read to write locks
                lock.unlock();
                auto write_lock = matrix_system.get_write_lock();
                std::atomic_signal_fence(std::memory_order_acquire);

                auto& symbol_table = matrix_system.Symbols();

                // With write-lock acquired, double check if we need to register new symbols:
                const uint64_t largest_existing_symbol = matrix_system.Symbols().size() - 1;
                if (largest_symbol > largest_existing_symbol) {
                    const bool can_be_nonhermitian = matrix_system.Context().can_be_nonhermitian();
                    symbol_table.create(largest_symbol - largest_existing_symbol, true, can_be_nonhermitian);
                }

                // Swap back to read lock
                write_lock.unlock();
                lock.lock();
            }
        }

        // Read (and simplify) inputs
        std::vector<Polynomial> polynomials;
        polynomials.reserve(input.inputPolynomials.size());
        for (const auto& input_poly: input.inputPolynomials) {
            polynomials.emplace_back(raw_data_to_polynomial(this->matlabEngine, poly_factory, input_poly));
        }



        // Export
        matlab::data::ArrayFactory factory;
        PolynomialExporter exporter{this->matlabEngine, factory,
                                    matrix_system.Context(), matrix_system.Symbols(), poly_factory.zero_tolerance};
        if (input.output_type == ImportPolynomialParams::OutputType::String) {
            matlab::data::StringArray string_out = factory.createArray<matlab::data::MATLABString>(input.input_shape);

            std::transform(polynomials.cbegin(), polynomials.cend(), string_out.begin(),
                           [&exporter](const Polynomial &poly) -> matlab::data::MATLABString {
                               return exporter.string(poly);
                           });
            output[0] = std::move(string_out);
        } else {
            matlab::data::CellArray cell_out = factory.createCellArray(input.input_shape);
            std::transform(polynomials.cbegin(), polynomials.cend(), cell_out.begin(),
                           [&exporter](const Polynomial &poly) -> matlab::data::CellArray {
                               return exporter.symbol_cell(poly);
                           });
            output[0] = std::move(cell_out);
        }

    }
}