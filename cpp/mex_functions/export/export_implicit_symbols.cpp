/**
 * export_implicit_symbols.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_implicit_symbols.h"

#include "scenarios/context.h"
#include "scenarios/inflation/canonical_observables.h"
#include "scenarios/inflation/inflation_implicit_symbols.h"
#include "scenarios/locality/locality_implicit_symbols.h"
#include "scenarios/locality/locality_operator_formatter.h"
#include "scenarios/locality/joint_measurement_iterator.h"

#include "matrix/operator_matrix.h"
#include "matrix/moment_matrix.h"

#include "error_codes.h"
#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"

namespace Moment::mex {

    namespace {

        matlab::data::SparseArray<double> combo_to_sparse_array(matlab::engine::MATLABEngine &engine,
                                                                matlab::data::ArrayFactory &factory,
                                                                const SymbolTable& table,
                                                                const SymbolCombo &combo) {
            // Special case, for completely zero matrix:
            const size_t nnz = combo.size();
            const size_t real_symbol_count = table.Basis.RealSymbolCount();
            if (nnz == 0) {
                return make_zero_sparse_matrix<double>(engine, {1, real_symbol_count});
            }

            auto rows_p = factory.createBuffer<size_t>(nnz);
            auto cols_p = factory.createBuffer<size_t>(nnz);
            auto data_p = factory.createBuffer<double>(nnz);

            // Blit data into the buffers
            size_t *rowsPtr = rows_p.get();
            size_t *colsPtr = cols_p.get();
            double *dataPtr = data_p.get();


            for (const auto [symbol_id, weight, conjugated]: combo) {
                *(rowsPtr++) = 0;
                auto [re_key, im_key] = table[symbol_id].basis_key();
                assert(re_key >= 0);
                assert(im_key < 0);
                *(colsPtr++) = re_key;
                *(dataPtr++) = weight;
            }

            return factory.createSparseArray<double>({1, real_symbol_count}, nnz,
                                                     std::move(data_p), std::move(rows_p), std::move(cols_p));
        }

        class LocalityImpliedSymbolWriter {
        private:
            matlab::engine::MATLABEngine &engine;
            matlab::data::ArrayFactory factory;

            const Locality::LocalityImplicitSymbols &implicitSymbols;
            const Locality::LocalityContext &context;
            const Locality::LocalityOperatorFormatter &formatter;
            const size_t implicit_table_length;
            const size_t real_symbol_count;
        public:
            matlab::data::StructArray output_array;

        private:
            size_t write_index = 0;

        public:
            LocalityImpliedSymbolWriter(matlab::engine::MATLABEngine &engine,
                                        const Locality::LocalityImplicitSymbols &impliedSymbols,
                                        const Locality::LocalityOperatorFormatter &formatter)
                    : engine{engine}, implicitSymbols{impliedSymbols}, context{impliedSymbols.context},
                      formatter{formatter},
                      implicit_table_length{implicitSymbols.Data().size() + 1},
                      real_symbol_count{implicitSymbols.symbols.Basis.RealSymbolCount()},
                      output_array{factory.createStructArray({1, implicit_table_length},
                                                             {"sequence", "indices", "real_coefficients"})} {

                // Add zero entry at front
                this->output_array[write_index]["sequence"] = factory.createScalar("0");
                this->output_array[write_index]["indices"] = factory.createArray<uint64_t>({0, 3});
                this->output_array[write_index]["real_coefficients"] = make_zero_sparse_matrix<double>(engine,
                                                                                   {1, real_symbol_count});
                ++write_index;
            }

            LocalityImpliedSymbolWriter(matlab::engine::MATLABEngine &engine,
                                        const Locality::LocalityImplicitSymbols &impliedSymbols,
                                        const Locality::LocalityOperatorFormatter &formatter,
                                        const std::span<const PMODefinition> symbols,
                                        const std::span<const Locality::PMIndex> indices)

                    : engine{engine}, implicitSymbols{impliedSymbols}, context{impliedSymbols.context},
                      formatter{formatter}, implicit_table_length{symbols.size()},
                      real_symbol_count{implicitSymbols.symbols.Basis.RealSymbolCount()},
                      output_array{factory.createStructArray({1, implicit_table_length},
                                                             {"sequence", "indices", "real_coefficients"})} {
                // Add entry
                this->operator()(symbols, indices);
            }

            void operator()(const std::span<const PMODefinition> symbols,
                            const std::span<const Locality::PMIndex> indices) {

                const size_t index_depth = indices.size();
                // Special case {} = ID
                if (indices.empty()) {
                    [[unlikely]]
                    this->output_array[write_index]["sequence"] = factory.createScalar("1");
                    this->output_array[write_index]["indices"] = factory.createArray<uint64_t>({0, 3});
                    this->output_array[write_index]["real_coefficients"]
                        = to_sparse_array(SymbolCombo{SymbolExpression{1, 1.}});
                    ++write_index;
                    return;
                }

                // First, create PMx indices
                std::vector<Locality::PMOIndex> indicesWithOutcomes;
                std::vector<uint64_t> entryIndices(index_depth * 3, 0);

                for (size_t i = 0; i < index_depth; ++i) {
                    indicesWithOutcomes.emplace_back(indices[i], 0);

                    entryIndices[i] = indices[i].party + 1; // matlab 1-indexing
                    entryIndices[index_depth + i] = indices[i].mmt + 1; // matlab 1-indexing
                }
                const matlab::data::ArrayDimensions indexArrayDim{indices.size(), 3};

                // Create iterator for reading out indices..
                OutcomeIndexIterator outputIndexIter{context.outcomes_per_measurement(indices)};

                // For each outcome of this joint measurement
                for (const auto &symbol: symbols) {
                    // Write PMO index data to array
                    const auto &outcomes = *outputIndexIter;
                    assert(outcomes.size() == indices.size());
                    for (size_t n = 0; n < outcomes.size(); ++n) {
                        entryIndices[2 * index_depth + n] = static_cast<uint64_t>(outcomes[n] + 1); // matlab 1-indexing
                        indicesWithOutcomes[n].outcome = outcomes[n];
                    }

                    matlab::data::TypedArray<uint64_t> index_array
                        = indices.empty() ? this->factory.createArray<uint64_t>(indexArrayDim)
                                          : this->factory.createArray(indexArrayDim,
                                                                      entryIndices.cbegin(), entryIndices.cend());

                    this->output_array[write_index]["sequence"] =
                            factory.createScalar(context.format_sequence(formatter, indicesWithOutcomes));
                    this->output_array[write_index]["indices"] = std::move(index_array);
                    this->output_array[write_index]["real_coefficients"] = to_sparse_array(symbol.expression);
                    ++write_index;
                    ++outputIndexIter;
                }
            }



        private:
            inline matlab::data::SparseArray<double> to_sparse_array(const SymbolCombo &combo) {
                return combo_to_sparse_array(this->engine, this->factory,
                                             this->implicitSymbols.symbols, combo);
            }
        };

        class InflationImpliedSymbolWriter {
        private:
            matlab::engine::MATLABEngine &engine;
            matlab::data::ArrayFactory factory;

            const Inflation::InflationImplicitSymbols &implicitSymbols;
            const Inflation::InflationContext &context;
            const Inflation::CanonicalObservables &canonicalObservables;
            const size_t real_symbol_count;


        public:
            InflationImpliedSymbolWriter(matlab::engine::MATLABEngine &engine,
                                         const Inflation::InflationImplicitSymbols &impliedSymbols)
                    : engine{engine}, implicitSymbols{impliedSymbols}, context{impliedSymbols.context},
                      canonicalObservables{impliedSymbols.canonicalObservables},
                      real_symbol_count{implicitSymbols.symbols.Basis.RealSymbolCount()} {
            }

            matlab::data::StructArray whole_table() {
                auto output = init_array(this->implicitSymbols.Data().size());

                size_t output_index = 0;
                size_t co_index = 0;

                for (auto chunk : implicitSymbols.BlockData()) {
                    assert(co_index < canonicalObservables.size());
                    const auto& canonical = canonicalObservables[co_index];
                    oper_name_t outcome_index = 0;

                    for (const auto& entry : chunk) {
                        write_row(output, output_index, canonical, outcome_index, entry);
                        ++output_index;
                        ++outcome_index;
                    }
                    ++co_index;
                }
                assert(co_index = canonicalObservables.size());

                return output;
            }

            matlab::data::StructArray one_observable(std::span<const Inflation::OVIndex> obsVarIndices) {
                const auto &observable = canonicalObservables.canonical(obsVarIndices);

                auto output = init_array(observable.outcomes);
                auto data_block = implicitSymbols.Block(observable.index);

                size_t output_index = 0;
                for (const auto& entry : data_block) {
                    write_row(output, output_index, observable, output_index, entry);
                    ++output_index;
                }

                return output;
            }

            matlab::data::StructArray one_outcome(std::span<const Inflation::OVOIndex> obsVarIndices) {
                const auto &observable = canonicalObservables.canonical(obsVarIndices);

                auto output = init_array(1);
                const auto data_block = implicitSymbols.Block(observable.index);
                const size_t outcome_index = context.flatten_outcome_index(obsVarIndices);
                if (outcome_index > data_block.size()) {
                    throw std::runtime_error{"Outcome index out of range."};
                }

                write_row(output, 0, observable, outcome_index, data_block[outcome_index]);

                return output;
            }

        private:
            matlab::data::StructArray init_array(const size_t table_length) {
                return factory.createStructArray({1, table_length}, {"sequence", "indices", "real_coefficients"});
            }

            void write_row(matlab::data::StructArray &output, const size_t output_index,
                           const Inflation::CanonicalObservable& canonical, const size_t outcome_index,
                           const PMODefinition& entry) {
                auto full_indices = context.unflatten_outcome_index(canonical.indices,
                                                                    static_cast<oper_name_t>(outcome_index));
                output[output_index]["sequence"] = factory.createScalar(context.format_sequence(full_indices));

                // Calculate indices
                if (full_indices.empty()) {
                    output[output_index]["indices"] = factory.createArray<uint64_t>({0, 3});
                } else {
                    const size_t index_size = full_indices.size();
                    auto index_data = factory.createBuffer<uint64_t>(3 * index_size);

                    size_t row = 0;
                    for (const auto &i: full_indices) {
                        index_data[row] = i.observable_variant.observable + 1;
                        index_data[index_size + row] = i.observable_variant.variant + 1;
                        index_data[2 * index_size + row] = i.outcome + 1;
                        ++row;
                    }
                    output[output_index]["indices"]
                            = factory.createArrayFromBuffer({index_size, 3}, std::move(index_data));
                }

                // And next, real co-efficients
                output[output_index]["real_coefficients"] =
                        combo_to_sparse_array(engine, factory, implicitSymbols.symbols, entry.expression);
            }
        };
    }

    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                     const Inflation::InflationImplicitSymbols &impliedSymbols) {
        InflationImpliedSymbolWriter iisw{engine, impliedSymbols};
        return iisw.whole_table();
    }


    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                     const Inflation::InflationImplicitSymbols &impliedSymbols,
                                                     std::span<const Inflation::OVIndex> obsVarIndices) {
        InflationImpliedSymbolWriter iisw{engine, impliedSymbols};
        return iisw.one_observable(obsVarIndices);
    }

    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                     const Inflation::InflationImplicitSymbols &impliedSymbols,
                                                     std::span<const Inflation::OVOIndex> obsVarIndices) {
        InflationImpliedSymbolWriter iisw{engine, impliedSymbols};
        return iisw.one_outcome(obsVarIndices);
    }


    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                               const Locality::LocalityOperatorFormatter& formatter,
                                               const Locality::LocalityImplicitSymbols& impliedSymbols) {
        LocalityImpliedSymbolWriter isw{engine, impliedSymbols, formatter};
        impliedSymbols.visit(isw);

        return std::move(isw.output_array);
    }


    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                     const Locality::LocalityOperatorFormatter& formatter,
                                                     const Locality::LocalityImplicitSymbols &impliedSymbols,
                                                     const std::span<const Locality::PMIndex> measurementIndex) {
        std::vector<size_t> globalMmtIndex;
        globalMmtIndex.reserve(measurementIndex.size());
        for (const auto& pmi : measurementIndex) {
            globalMmtIndex.emplace_back(pmi.global_mmt);
        }

        auto pmod = impliedSymbols.get(globalMmtIndex);

        LocalityImpliedSymbolWriter isw{engine, impliedSymbols, formatter, pmod, measurementIndex};
        return std::move(isw.output_array);
    }

    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                     const Locality::LocalityOperatorFormatter& formatter,
                                                     const Locality::LocalityImplicitSymbols &impliedSymbols,
                                                     const std::span<const Locality::PMOIndex> outcomeIndex) {
        matlab::data::ArrayFactory factory;

        const auto& context = impliedSymbols.context;

        // Find element...
        const auto& symbolDefinition = impliedSymbols.get(outcomeIndex);

        // (Re)make indices for export
        const size_t index_depth = outcomeIndex.size();
        std::vector<uint64_t> entryIndices(index_depth * 3, 0);
        for (size_t i = 0; i < index_depth; ++i) {
            entryIndices[i] = outcomeIndex[i].party + 1;                       // matlab 1-indexing
            entryIndices[index_depth + i] = outcomeIndex[i].mmt + 1;           // matlab 1-indexing
            entryIndices[2*index_depth + i ] = outcomeIndex[i].outcome + 1;    // matlab 1-indexing
        }
        const matlab::data::ArrayDimensions indexArrayDim{outcomeIndex.size(), 3};
        matlab::data::TypedArray<uint64_t> index_array = outcomeIndex.empty()
                                                         ? factory.createArray<uint64_t>(indexArrayDim)
                                                         : factory.createArray(indexArrayDim,
                                                                               entryIndices.cbegin(),
                                                                               entryIndices.cend());

        // Write entry
        auto output = factory.createStructArray({1, 1}, {"sequence", "indices", "real_coefficients"});
        output[0]["sequence"] = factory.createScalar(context.format_sequence(formatter, outcomeIndex));
        output[0]["indices"] = std::move(index_array);
        output[0]["real_coefficients"] = combo_to_sparse_array(engine, factory, impliedSymbols.symbols,
                                                               symbolDefinition.expression);
        return output;
    }
}