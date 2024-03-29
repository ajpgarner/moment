/**
 * list.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "list.h"

#include "matrix_system/matrix_system.h"

#include "matrix/monomial_matrix.h"

#include "scenarios/context.h"

#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "storage_manager.h"

#include <cassert>

namespace Moment::mex::functions {
    namespace {
        void output_ms_info(std::ostream& raw_os, uint32_t id,
                            const MatrixSystem& ms,
                            bool export_context, bool export_symbols, bool export_mat_props,
                            const EnvironmentalVariables& env) {
            auto read_lock = ms.get_read_lock();
            const auto& context = ms.Context();
            const auto& symbols = ms.Symbols();
            ContextualOS os{raw_os, context, symbols};
            auto lf = env.get_locality_formatter();
            os.format_info.locality_formatter = lf.get();

            os << "System #" << id << ": " << ms.system_type_name() << ": ";
            const size_t operator_count = context.size();
            os << operator_count << " " << (operator_count != 1 ? "operators" : "operator") << ", ";
            const size_t symbol_count = symbols.size();
            os << symbol_count << " " << (symbol_count != 1 ? "symbols" : "symbol") << ", ";
            const size_t matrix_count = ms.size();
            os << matrix_count << " " << (matrix_count != 1 ? "matrices" : "matrix") << ", ";
            const size_t rb_count = ms.Rulebook.size();
            os << rb_count <<  " " << (matrix_count != 1 ? "rulebooks" : "rulebook") << ".";

            os << "\nPOLYNOMIAL FACTORY:\n " << ms.polynomial_factory();

            if (export_context) {
                os << "\nCONTEXT:\n " << context;
            }

            if (matrix_count > 0) {
                os << "\nMATRICES:";
                for (size_t matrix_index = 0; matrix_index < matrix_count; ++matrix_index) {
                    const auto &matrix = ms[matrix_index];
                    os << "\n " << matrix_index << ": " << matrix.Dimension() << "x" << matrix.Dimension();
                    os << " " << matrix.Description();
                    if (export_mat_props) {
                        os << "\n";
                        const auto num_us = matrix.IncludedSymbols().size();
                        os << " with "
                           << num_us << " unique " << (num_us != 1 ? "symbols" : "symbol");
                        const auto num_re = matrix.RealBasisIndices().size();
                        if (num_re > 0) {
                            os << ", " << num_re << " real";
                        }
                        const auto num_im = matrix.ImaginaryBasisIndices().size();
                        if (num_im > 0) {
                            os << ", " << num_im << " imaginary";
                        }
                    }
                }
            }

            if (rb_count > 0) {
                os << "\nRULEBOOKS:";
                for (size_t rb_index = 0; rb_index < rb_count; ++rb_index) {
                    const auto& rulebook = ms.Rulebook(rb_index);
                    size_t rule_count = rulebook.size();
                    os << "\n " << rb_index << ": " << rule_count << " " << (rule_count != 1 ? "rules" : "rule") << ": "
                       << rulebook.name();
                }
            }

            if (export_symbols) {
                os << "\n" << ms.Symbols();
            }
        }
    }

    ListParams::ListParams(SortedInputs &&inputs)
        : SortedInputs(std::move(inputs)) {
        if (this->inputs.size() >= 1) {
            this->output_type = OutputType::OneSystem;
            this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);
        } else {
            this->output_type = OutputType::All;
        }
        this->structured = this->flags.contains(u"structured");
        this->export_contexts = this->flags.contains(u"contexts");
        this->export_symbols = this->flags.contains(u"symbols");
        this->export_matrix_properties = this->flags.contains(u"details");
    }

    List::List(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 0;
        this->max_inputs = 1;
        this->min_outputs = 0;
        this->max_outputs = 1;
        this->flag_names.insert(u"structured");
        this->flag_names.insert(u"context");
        this->flag_names.insert(u"symbols");
        this->flag_names.insert(u"details");

        this->mutex_params.add_mutex(u"structured", u"context");
        this->mutex_params.add_mutex(u"structured", u"details");
        this->mutex_params.add_mutex(u"structured", u"symbols");
    }


    void List::extra_input_checks(ListParams &input) const {
        if ((input.output_type == ListParams::OutputType::OneSystem)
            && (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key))) {
            throw BadParameter{"Invalid or expired reference to MomentMatrix."};
        }
    }


    void List::operator()(IOArgumentRange output, ListParams &input) {
        bool output_to_console = (output.size() == 0);

        // If verbose flag, override export flags
        bool generate_string = !input.structured;
        if (this->verbose) {
            input.export_contexts = true;
            input.export_symbols = true;
            input.export_matrix_properties = true;
            generate_string = true;
            // In verbose mode, always output string
            if (input.structured) {
                output_to_console = true;
            }
        }

        // Make string info, if required
        std::string list_as_str;
        if (generate_string) {
            list_as_str = generateListString(input);
        }

        if (output_to_console) {
            list_as_str.append("\n");
            print_to_console(this->matlabEngine, list_as_str);
        }

        if (output.size() > 0) {
            matlab::data::ArrayFactory factory;
            if (input.structured) {
                if (input.output_type == ListParams::OutputType::OneSystem) {
                    output[0] = generateOneSystemStruct(input);
                } else {
                    output[0] = generateListStruct();
                }
            } else {
                output[0] = factory.createScalar(list_as_str);
            }
        }
    }

    std::string List::generateListString(const ListParams &input) const {
        std::stringstream ss;
        if (input.output_type == ListParams::OutputType::All) {
            auto [id, msPtr] = storageManager.MatrixSystems.first();
            bool done_one = false;
            if ((id != 0xffffffff) && (msPtr != nullptr)) {
                do {
                    if (done_one) {
                        ss << "\n";
                    } else {
                        done_one = true;
                    }
                    output_ms_info(ss, id, *msPtr,
                                   input.export_contexts, input.export_symbols, input.export_matrix_properties,
                                   *this->settings);
                    // Get next:
                    auto [next_id, nextPtr] = storageManager.MatrixSystems.next(id);
                    id = next_id;
                    msPtr = nextPtr;
                } while ((id != 0xffffffff) && (msPtr != nullptr));
            } else {
                ss << "No matrix systems defined.";
            }
        } else {
            auto id = PersistentStorage<MatrixSystem>::get_index(input.matrix_system_key);
            auto msPtr = storageManager.MatrixSystems.get(input.matrix_system_key);
            output_ms_info(ss, id, *msPtr,
                           input.export_contexts, input.export_symbols, input.export_matrix_properties,
                           *this->settings);
        }
        return ss.str();
    }

    matlab::data::StructArray List::generateListStruct() const {
        matlab::data::ArrayFactory factory;
        struct temp_system_info_t {
            uint64_t id;
            std::string desc;
            uint64_t operators;
            uint64_t matrices;
            uint64_t symbols;
            uint64_t rulebooks;

            temp_system_info_t(uint64_t id, std::string desc, uint64_t operators, uint64_t matrices, uint64_t symbols, uint64_t rbs)
                : id{id}, desc{std::move(desc)},
                operators{operators}, matrices{matrices}, symbols{symbols}, rulebooks{rbs} { }
        };
        std::vector<temp_system_info_t> data;

        auto [id, msPtr] = storageManager.MatrixSystems.first();
        while ((id != 0xffffffff) && (msPtr != nullptr)) {
            auto lock = msPtr->get_read_lock();

            data.emplace_back(storageManager.MatrixSystems.sign_index(id), msPtr->system_type_name(),
                              msPtr->Context().size(),
                              msPtr->size(), msPtr->Symbols().size(), msPtr->Rulebook.size());

            lock.unlock();

            // Get next:
            auto [next_id, nextPtr] = storageManager.MatrixSystems.next(id);
            id = next_id;
            msPtr = nextPtr;
        }

        matlab::data::ArrayDimensions dimensions{1, data.size()};
        auto output = factory.createStructArray(std::move(dimensions),
                                                {"RefId", "Description", "OperatorCount",
                                                 "Matrices", "Symbols", "Rulebooks"});
        size_t out_index = 0;
        for (const auto& datum : data) {
            output[out_index]["RefId"] = factory.createScalar(datum.id);
            output[out_index]["Description"] = factory.createScalar(datum.desc);
            output[out_index]["OperatorCount"] = factory.createScalar(datum.operators);
            output[out_index]["Matrices"] = factory.createScalar(datum.matrices);
            output[out_index]["Symbols"] = factory.createScalar(datum.symbols);
            output[out_index]["Rulebooks"] = factory.createScalar(datum.rulebooks);
            ++out_index;
        }
        return output;
    }

    matlab::data::StructArray List::generateOneSystemStruct(const ListParams &input) const {
        auto msPtr = storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr);
        auto lock = msPtr->get_read_lock();

        matlab::data::ArrayFactory factory;
        auto output = factory.createStructArray({1, 1}, {"RefId", "Description", "OperatorCount",
                                                         "Matrices", "Symbols", "Rulebooks"});
        output[0]["RefId"] = factory.createScalar(input.matrix_system_key);
        output[0]["Description"] = factory.createScalar(msPtr->system_type_name());
        output[0]["OperatorCount"] = factory.createScalar(msPtr->Context().size());
        output[0]["Matrices"] = factory.createScalar(msPtr->size());
        output[0]["Symbols"] = factory.createScalar(msPtr->Symbols().size());
        output[0]["Rulebooks"] = factory.createScalar(msPtr->Rulebook.size());
        return output;
    }
}