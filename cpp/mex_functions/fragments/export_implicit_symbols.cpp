/**
 * export_implicit_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_implicit_symbols.h"

#include "operators/context.h"
#include "operators/implicit_symbols.h"
#include "operators/joint_measurement_iterator.h"
#include "operators/moment_matrix.h"

#include "utilities/make_sparse_matrix.h"

namespace NPATK::mex {

    namespace {

        class ImpliedSymbolWriter {
        private:
            matlab::engine::MATLABEngine& engine;
            matlab::data::ArrayFactory factory;

            const ImplicitSymbols& implicitSymbols;
            const Context& context;
            const size_t implicit_table_length;
            const size_t real_symbol_count;
        public:
            matlab::data::StructArray output_array;


        private:
            size_t write_index = 0;
        public:
            ImpliedSymbolWriter(matlab::engine::MATLABEngine& engine,
                                const ImplicitSymbols &impliedSymbols)
                : engine{engine}, implicitSymbols{impliedSymbols}, context{implicitSymbols.context},
                  implicit_table_length{implicitSymbols.Data().size() + 1},
                  real_symbol_count{implicitSymbols.momentMatrix.BasisIndices().RealSymbols().size()},
                  output_array{factory.createStructArray({1, implicit_table_length},
                                                         {"sequence", "indices", "real_coefficients"})} {

                // Add zero entry at front
                this->output_array[write_index]["sequence"] = factory.createScalar("0");
                this->output_array[write_index]["indices"] = factory.createArray<uint64_t>({0, 3});
                this->output_array[write_index]["real_coefficients"] = make_zero_sparse_matrix<double>(engine,
                                                                                             {1, real_symbol_count});
                ++write_index;
            }

            void operator()(const std::span<const ImplicitSymbols::PMODefinition> symbols,
                            const std::span<const PMIndex> indices) {

                const size_t index_depth = indices.size();
                // Special case {} = ID
                if (indices.empty()) {
                    [[unlikely]]
                    this->output_array[write_index]["sequence"] = factory.createScalar("1");
                    this->output_array[write_index]["indices"] = factory.createArray<uint64_t>({0, 3});
                    this->output_array[write_index]["real_coefficients"] = to_sparse_array(SymbolCombo{{1, 1.}});
                    ++write_index;
                    return;
                }

                // First, create PMx indices
                std::vector<PMOIndex> indicesWithOutcomes;
                std::vector<uint64_t> entryIndices(index_depth * 3, 0);

                for (size_t i = 0; i < index_depth; ++i) {
                    indicesWithOutcomes.emplace_back(indices[i], 0);

                    entryIndices[i] = indices[i].party + 1; // matlab 1-indexing
                    entryIndices[index_depth + i] = indices[i].mmt + 1; // matlab 1-indexing
                }
                const matlab::data::ArrayDimensions indexArrayDim{indices.size(), 3};

                // Create iterator for reading out indices..
                OutcomeIndexIterator outputIndexIter{context, indices};

                // For each outcome of this joint measurement
                for (const auto & symbol : symbols) {
                    // Write PMO index data to array
                    const auto& outcomes = *outputIndexIter;
                    assert(outcomes.size() == indices.size());
                    for (size_t n = 0; n < outcomes.size(); ++n) {
                        entryIndices[2*index_depth + n] = static_cast<uint64_t>(outcomes[n] + 1); // matlab 1-indexing
                        indicesWithOutcomes[n].outcome = outcomes[n];
                    }

                    matlab::data::TypedArray<uint64_t> index_array = indices.empty()
                            ? this->factory.createArray<uint64_t>(indexArrayDim)
                            : this->factory.createArray(indexArrayDim, entryIndices.cbegin(), entryIndices.cend());

                    this->output_array[write_index]["sequence"] =
                            factory.createScalar(context.format_sequence(indicesWithOutcomes));
                    this->output_array[write_index]["indices"] = std::move(index_array);
                    this->output_array[write_index]["real_coefficients"] = to_sparse_array(symbol.expression);
                    ++write_index;
                    ++outputIndexIter;
                }
            }


        private:
            matlab::data::SparseArray<double> to_sparse_array(const SymbolCombo& combo) {
                // Special case, for completely zero matrix:
                const size_t nnz = combo.size();
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


                for (const auto [symbol_id, weight] : combo) {
                //std::for_each(combo.begin(), combo.end(), [&](const SymbolCombo::column_value_t &e) {
                    *(rowsPtr++) = 0;
                    auto [re_key, im_key] = this->implicitSymbols.momentMatrix.BasisIndices().BasisKey(symbol_id);
                    assert(re_key >= 0);
                    assert(im_key < 0);
                    *(colsPtr++) = re_key;
                    *(dataPtr++) = weight;
                }

                return factory.createSparseArray<double>({1, real_symbol_count}, nnz,
                                                         std::move(data_p), std::move(rows_p), std::move(cols_p));
            }
        };
    }

    matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                               const ImplicitSymbols &impliedSymbols) {
        ImpliedSymbolWriter isw{engine, impliedSymbols};
        impliedSymbols.visit(isw);

        return std::move(isw.output_array);
    }

}
