/**
 * generate_basis.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "generate_basis.h"

#include "../helpers/enumerate_symbols.h"
#include "../helpers/export_basis_key.h"
#include "../helpers/make_sparse_matrix.h"
#include "../helpers/reporting.h"
#include "../helpers/visitor.h"

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

                if (this->imp.Type() == IndexMatrixProperties::BasisType::Hermitian) {
                    for (size_t ri = 0, ri_max = this->imp.ImaginarySymbols().size(); ri < ri_max; ++ri) {
                        output.second[ri] = factory.createArray<std::complex<double>>({this->imp.Dimension(), this->imp.Dimension()});
                    }
                }
                return output;
            }

        };

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


        CellArrayPair make_dense_basis(matlab::engine::MATLABEngine &engine,
                                                 const matlab::data::Array &input,
                                                 const IndexMatrixProperties &imp) {
            // Get symbols in matrix...
            MakeDenseBasisVisitor visitor{engine, imp};
            VisitDispatcher dispatcher{engine, visitor};
            return dispatcher(input);
        }

        CellArrayPair make_sparse_basis(matlab::engine::MATLABEngine &engine,
                                                  const matlab::data::Array &input,
                                                  const IndexMatrixProperties &imp) {
            // Get symbols in matrix...
            MakeSparseBasisVisitor visitor{engine, imp};
            VisitDispatcher dispatcher{engine, visitor};
            return dispatcher(input);
        }
    }


    GenerateBasis::GenerateBasis(matlab::engine::MATLABEngine &matlabEngine)
        : MexFunction(matlabEngine, MEXEntryPointID::GenerateBasis, u"generate_basis") {
        this->min_inputs = this->max_inputs = 1;
        this->min_outputs = 1;
        this->max_outputs = 3;
        this->flag_names.emplace(u"symmetric");
        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");


    }

    std::pair<bool, std::basic_string<char16_t>> GenerateBasis::validate_inputs(const SortedInputs &input) const {
        bool is_sym = input.flags.contains(u"symmetric");
        bool is_herm = input.flags.contains(u"hermitian");
        if (is_sym && is_herm) {
            return {false, u"Only one of \"symmetric\" or \"hermitian\" may be set."};
        }

        bool is_sparse = input.flags.contains(u"sparse");
        bool is_dense = input.flags.contains(u"dense");
        if (is_sparse && is_dense) {
            return {false, u"Only one of \"sparse\" or \"dense\" may be set."};
        }


        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            return {false, u"Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            return {false, u"Input must be a square matrix."};
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
                break;
            default:
                return {false, u"Matrix type must be numeric."};
        }

        // TODO: Check for symmetry/hermiticity

        return {true, u""};
    }

    void GenerateBasis::operator()(FlagArgumentRange output, SortedInputs &&input) {
        bool debug = input.flags.contains(u"debug");
        bool verbose = debug || input.flags.contains(u"verbose");

        IndexMatrixProperties::BasisType basis_type = IndexMatrixProperties::BasisType::Symmetric;
        if (input.flags.contains(u"symmetric")) {
            basis_type = IndexMatrixProperties::BasisType::Symmetric;
        } else if (input.flags.contains(u"hermitian")) {
            basis_type = IndexMatrixProperties::BasisType::Hermitian;
        }

        // Hermitian output requires two outputs...
        if ((basis_type == IndexMatrixProperties::BasisType::Hermitian) && (output.size() < 2)) {
            throw_error(this->matlabEngine, std::string("When generating a Hermitian basis, two outputs are required (one for ")
                                          + "symmetric basis elements associated with the real components, one for the "
                                          + "anti-symmetric imaginary elements associated with the imaginary components.");
        }

        // Symmetric output cannot have three outputs...
        if ((basis_type == IndexMatrixProperties::BasisType::Symmetric) && (output.size() > 2)) {
            throw_error(this->matlabEngine, std::to_string(output.size()) + " outputs supplied for symmetric basis output, but only"
                                                        + " two will be generated (basis, and key).");
        }


        // Default sparsity to matrix type of input, but allow for override
        bool sparse_output = (input.inputs[0].getType() == matlab::data::ArrayType::SPARSE_DOUBLE);
        if (input.flags.contains(u"sparse")) {
            sparse_output = true;
        } else if (input.flags.contains(u"dense")) {
            sparse_output = false;
        }

        auto matrix_properties = enumerate_symbols(matlabEngine, input.inputs[0], basis_type, debug);

        if (verbose) {
            std::stringstream ss;
            ss << "Found " << matrix_properties.BasisMap().size() << " unique symbols ["
               << matrix_properties.RealSymbols().size() << " with real parts, "
               << matrix_properties.ImaginarySymbols().size() << " with imaginary parts].\n";
            if (debug) {
                ss << "Outputting as " << (sparse_output ? "sparse" : "dense") << " basis.\n";
            }
            print_to_console(matlabEngine, ss.str());
        }


        if (sparse_output) {
            auto [sym, anti_sym] = make_sparse_basis(this->matlabEngine, input.inputs[0], matrix_properties);
            output[0] = std::move(sym);
            if (basis_type == IndexMatrixProperties::BasisType::Hermitian) {
                output[1] = std::move(anti_sym);
            }
        } else {
            auto [sym, anti_sym] = make_dense_basis(this->matlabEngine, input.inputs[0], matrix_properties);
            output[0] = std::move(sym);
            if (basis_type == IndexMatrixProperties::BasisType::Hermitian) {
                output[1] = std::move(anti_sym);
            }
        }

        // If enough outputs supplied, also provide keys
        ptrdiff_t key_output = (basis_type == IndexMatrixProperties::BasisType::Hermitian) ? 2 : 1;
        if (output.size() > key_output) {
            output[key_output] = export_basis_key(this->matlabEngine, matrix_properties);
        }
    }
}