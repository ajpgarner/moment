/**
 * generate_basis.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "generate_basis.h"
#include "../mex_main.h"

#include "../fragments/enumerate_symbols.h"
#include "../fragments/export_basis_key.h"
#include "../utilities/make_sparse_matrix.h"
#include "../utilities/reporting.h"
#include "../utilities/visitor.h"
#include "fragments/read_symbol_or_fail.h"

#include <complex>

namespace NPATK::mex::functions {

    using CellArrayPair = std::pair<matlab::data::CellArray, matlab::data::CellArray>;

    namespace {
        struct MakeDenseBasisVisitor {
        private:
            matlab::engine::MATLABEngine &engine;
            const IndexMatrixProperties &imp;

        public:
            using return_type = CellArrayPair;

            MakeDenseBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                                  const IndexMatrixProperties &matrix_properties)
                    : engine(engineRef), imp(matrix_properties) {}


            /** Dense input -> dense output */
            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                auto output = create_empty_basis();

                for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                    for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                        NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                        auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                        if (re_id>=0) {
                            matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                            re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                            re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                        }
                        if (im_id>=0) {
                            matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                            im_mat[index_i][index_j] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                            im_mat[index_j][index_i] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                        }
                    }
                }

                return output;
            }


            /** String input -> dense output */
            return_type string(const matlab::data::StringArray& matrix) {
                auto output = create_empty_basis();
                for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                    for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                        SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                        auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                        if (re_id>=0) {
                            matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                            re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                            re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                        }
                        if (im_id>=0) {
                            matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                            im_mat[index_i][index_j] = std::complex<double>{
                                    0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                            im_mat[index_j][index_i] = std::complex<double>{
                                    0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                        }
                    }
                }

                return output;
            }

            /** Sparse input -> dense output */
            template<std::convertible_to<symbol_name_t> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {
                auto output = create_empty_basis();

                auto iter = matrix.cbegin();
                while (iter != matrix.cend()) {
                    auto indices = matrix.getIndex(iter);

                    if (indices.first > indices.second) {
                        ++iter;
                        continue;
                    }

                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);
                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[indices.first][indices.second] = elem.negated ? -1 : 1;
                        re_mat[indices.second][indices.first] = elem.negated ? -1 : 1;
                    }
                    if (im_id>=0) {
                        matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                        im_mat[indices.first][indices.second] = std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        im_mat[indices.second][indices.first] = std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                    ++iter;
                }

                return output;
            }

        private:

            CellArrayPair create_empty_basis() {
                matlab::data::ArrayFactory factory{};

                matlab::data::ArrayDimensions re_ad{this->imp.RealSymbols().empty() ? 0U : 1U,
                                                    this->imp.RealSymbols().size()};
                matlab::data::ArrayDimensions im_ad{this->imp.ImaginarySymbols().empty() ? 0U : 1U,
                                                    this->imp.ImaginarySymbols().size()};

                auto output = std::make_pair(factory.createCellArray(re_ad),
                                             factory.createCellArray(im_ad));

                for (size_t rr = 0, rr_max = this->imp.RealSymbols().size(); rr < rr_max; ++rr) {
                    output.first[rr] = factory.createArray<double>({this->imp.Dimension(), this->imp.Dimension()});
                }

                if (this->imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) {
                    for (size_t ri = 0, ri_max = this->imp.ImaginarySymbols().size(); ri < ri_max; ++ri) {
                        output.second[ri] = factory.createArray<std::complex<double>>({this->imp.Dimension(), this->imp.Dimension()});
                    }
                }
                return output;
            }

        };

        static_assert(concepts::VisitorHasRealDense<MakeDenseBasisVisitor>);
        static_assert(concepts::VisitorHasRealSparse<MakeDenseBasisVisitor>);
        static_assert(concepts::VisitorHasString<MakeDenseBasisVisitor>);

        struct MakeSparseBasisVisitor {
        public:
            using return_type = CellArrayPair;

        private:
            matlab::engine::MATLABEngine &engine;
            const IndexMatrixProperties &imp;

            struct sparse_basis_re_frame {
                std::vector<size_t> index_i{};
                std::vector<size_t> index_j{};
                std::vector<double> values{};

                void push_back(size_t i, size_t j, double value) {
                    index_i.emplace_back(i);
                    index_j.emplace_back(j);
                    values.emplace_back(value);

                    if (i != j) {
                        index_i.emplace_back(j);
                        index_j.emplace_back(i);
                        values.emplace_back(value);
                    }
                }
            };

            struct sparse_basis_im_frame {
                std::vector<size_t> index_i{};
                std::vector<size_t> index_j{};
                std::vector<std::complex<double>> values;

                void push_back(size_t i, size_t j, std::complex<double> value) {
                    index_i.emplace_back(i);
                    index_j.emplace_back(j);
                    values.emplace_back(value);

                    if (i != j) {
                        index_i.emplace_back(j);
                        index_j.emplace_back(i);
                        values.emplace_back(conj(value));
                    }
                }
            };

        public:
            MakeSparseBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                                   const IndexMatrixProperties &matrix_properties)
                    : engine(engineRef), imp(matrix_properties) {}


            /** Dense input -> sparse output */
            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                std::vector<sparse_basis_re_frame> real_basis_frame(this->imp.RealSymbols().size());
                std::vector<sparse_basis_im_frame> im_basis_frame(this->imp.ImaginarySymbols().size());

                for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                    for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                        NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                        auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                        if (re_id>=0) {
                            assert(re_id < real_basis_frame.size());
                            real_basis_frame[re_id].push_back(index_i, index_j, elem.negated ? -1. : 1.);
                        }

                        if (im_id>=0) {
                            assert(im_id < im_basis_frame.size());
                            im_basis_frame[im_id].push_back(index_i, index_j, std::complex<double>{
                                0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                        }
                    }
                }

                return create_basis(real_basis_frame, im_basis_frame);
            }

            /** String (utf16) input -> sparse output */
            return_type string(const matlab::data::StringArray &matrix) {
                std::vector<sparse_basis_re_frame> real_basis_frame(this->imp.RealSymbols().size());
                std::vector<sparse_basis_im_frame> im_basis_frame(this->imp.ImaginarySymbols().size());

                for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                    for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                        SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                        auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                        if (re_id >= 0) {
                            assert(re_id < real_basis_frame.size());
                            real_basis_frame[re_id].push_back(index_i, index_j, elem.negated ? -1. : 1.);
                        }

                        if (im_id >= 0) {
                            assert(im_id < im_basis_frame.size());
                            im_basis_frame[im_id].push_back(index_i, index_j, std::complex<double>{
                                    0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                        }
                    }
                }
                return create_basis(real_basis_frame, im_basis_frame);
            }

            /** Sparse input -> sparse output */
            template<std::convertible_to<symbol_name_t> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {
                std::vector<sparse_basis_re_frame> real_basis_frame(this->imp.RealSymbols().size());
                std::vector<sparse_basis_im_frame> im_basis_frame(this->imp.ImaginarySymbols().size());

                std::stringstream ss;
                auto iter = matrix.cbegin();
                while (iter != matrix.cend()) {
                    auto [row, col] = matrix.getIndex(iter);
                    if (row > col) {
                        ++iter;
                        continue;
                    }

                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};

                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        assert(re_id < real_basis_frame.size());
                        real_basis_frame[re_id].push_back(row, col,
                                                          elem.negated ? -1. : 1.);
                    }
                    if (im_id>=0) {
                        assert(im_id < im_basis_frame.size());
                        im_basis_frame[im_id].push_back(row, col, std::complex<double>(
                                0.0, (elem.negated != elem.conjugated) ? -1. : 1.));

                    }
                    ++iter;
                }


                return create_basis(real_basis_frame, im_basis_frame);
            }

        private:
            CellArrayPair create_basis(const std::vector<sparse_basis_re_frame>& re_frame,
                                       const std::vector<sparse_basis_im_frame>& im_frame) {
                matlab::data::ArrayFactory factory{};
                matlab::data::ArrayDimensions re_ad{re_frame.empty() ? 0U : 1U,
                                                    re_frame.size()};
                matlab::data::ArrayDimensions im_ad{im_frame.empty() ? 0U : 1U,
                                                    im_frame.size()};
                CellArrayPair output = std::make_pair(factory.createArray<matlab::data::Array>(re_ad),
                                                      factory.createArray<matlab::data::Array>(im_ad));

                for (size_t re_id = 0, re_max = re_frame.size(); re_id < re_max; ++re_id) {

                    output.first[0][re_id] = make_sparse_matrix<double>(factory,
                                                                        std::make_pair(this->imp.Dimension(),
                                                                                       this->imp.Dimension()),
                                                             re_frame[re_id].index_i,
                                                             re_frame[re_id].index_j,
                                                             re_frame[re_id].values);
                }

                for (size_t im_id = 0, im_max = im_frame.size(); im_id < im_max; ++im_id) {
                    output.second[0][im_id] = make_sparse_matrix<std::complex<double>>(factory,
                                                                            std::make_pair(this->imp.Dimension(),
                                                                                           this->imp.Dimension()),
                                                              im_frame[im_id].index_i,
                                                              im_frame[im_id].index_j,
                                                              im_frame[im_id].values);
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<MakeSparseBasisVisitor>);
        static_assert(concepts::VisitorHasRealSparse<MakeSparseBasisVisitor>);
        static_assert(concepts::VisitorHasString<MakeSparseBasisVisitor>);

        CellArrayPair make_dense_basis(matlab::engine::MATLABEngine &engine,
                                                 const matlab::data::Array &input,
                                                 const IndexMatrixProperties &imp) {
            // Get symbols in matrix...
            return DispatchVisitor(engine, input, MakeDenseBasisVisitor{engine, imp});
        }

        CellArrayPair make_sparse_basis(matlab::engine::MATLABEngine &engine,
                                                  const matlab::data::Array &input,
                                                  const IndexMatrixProperties &imp) {
            // Get symbols in matrix...
            return DispatchVisitor(engine, input, MakeSparseBasisVisitor{engine, imp});
        }
    }


    GenerateBasis::GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
        : MexFunction(matlabEngine, storage, MEXEntryPointID::GenerateBasis, u"generate_basis") {
        this->min_inputs = this->max_inputs = 1;
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"symmetric");
        this->flag_names.emplace(u"hermitian");
        this->mutex_params.add_mutex(u"symmetric", u"hermitian");

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"dense", u"sparse");


    }

    GenerateBasisParams::GenerateBasisParams(NPATK::mex::SortedInputs &&structuredInputs)
        : SortedInputs(std::move(structuredInputs)) {
        // Determine sparsity of output
        this->sparse_output = (inputs[0].getType() == matlab::data::ArrayType::SPARSE_DOUBLE);
        if (flags.contains(u"sparse")) {
            this->sparse_output = true;
        } else if (flags.contains(u"dense")) {
            this->sparse_output = false;
        }
    }

    std::unique_ptr<SortedInputs> GenerateBasis::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            throw errors::BadInput{errors::bad_param, "Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            throw errors::BadInput{errors::bad_param, "Input must be a square matrix."};
        }

        switch(input.inputs[0].getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                throw errors::BadInput{errors::bad_param, "Matrix type must be numeric."};
        }

        // TODO: Check for symmetry/hermiticity?

        return std::make_unique<GenerateBasisParams>(std::move(input));
    }

    void GenerateBasis::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<GenerateBasisParams&>(*inputPtr);

        IndexMatrixProperties::MatrixType basis_type = IndexMatrixProperties::MatrixType::Symmetric;
        if (input.flags.contains(u"symmetric")) {
            basis_type = IndexMatrixProperties::MatrixType::Symmetric;
        } else if (input.flags.contains(u"hermitian")) {
            basis_type = IndexMatrixProperties::MatrixType::Hermitian;
        }

        // Hermitian output requires two outputs...
        if ((basis_type == IndexMatrixProperties::MatrixType::Hermitian) && (output.size() < 2)) {
            throw_error(this->matlabEngine, errors::too_few_outputs,
                                        std::string("When generating a Hermitian basis, two outputs are required (one for ")
                                          + "symmetric basis elements associated with the real components, one for the "
                                          + "anti-symmetric imaginary elements associated with the imaginary components.");
        }

        // Symmetric output cannot have three outputs...
        if ((basis_type == IndexMatrixProperties::MatrixType::Symmetric) && (output.size() > 2)) {
            throw_error(this->matlabEngine, errors::too_many_outputs,
                                            std::to_string(output.size()) + " outputs supplied for symmetric basis output, but only"
                                                        + " two will be generated (basis, and key).");
        }


        auto matrix_properties = enumerate_upper_symbols(matlabEngine, input.inputs[0], basis_type, debug);

        if (verbose) {
            std::stringstream ss;
            ss << "Found " << matrix_properties.BasisMap().size() << " unique symbols ["
               << matrix_properties.RealSymbols().size() << " with real parts, "
               << matrix_properties.ImaginarySymbols().size() << " with imaginary parts].\n";
            if (debug) {
                ss << "Outputting as " << (input.sparse_output ? "sparse" : "dense") << " basis.\n";
            }
            print_to_console(matlabEngine, ss.str());
        }

        if (input.sparse_output) {
            auto [sym, anti_sym] = make_sparse_basis(this->matlabEngine, input.inputs[0], matrix_properties);
            output[0] = std::move(sym);
            if (basis_type == IndexMatrixProperties::MatrixType::Hermitian) {
                output[1] = std::move(anti_sym);
            }
        } else {
            auto [sym, anti_sym] = make_dense_basis(this->matlabEngine, input.inputs[0], matrix_properties);
            output[0] = std::move(sym);
            if (basis_type == IndexMatrixProperties::MatrixType::Hermitian) {
                output[1] = std::move(anti_sym);
            }
        }

        // If enough outputs supplied, also provide keys
        ptrdiff_t key_output = (basis_type == IndexMatrixProperties::MatrixType::Hermitian) ? 2 : 1;
        if (output.size() > key_output) {
            output[key_output] = export_basis_key(this->matlabEngine, matrix_properties);
        }
    }
}
