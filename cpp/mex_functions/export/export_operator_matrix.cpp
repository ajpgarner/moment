/**
 * export_operator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_operator_matrix.h"

#include "export_operator_sequence.h"
#include "export_polynomial.h"

#include "matrix_system/matrix_system.h"
#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "symbolic/polynomial_factory.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "utilities/utf_conversion.h"

#include "errors.h"
#include "utilities/reporting.h"



namespace Moment::mex {

    namespace {
        class WriteSymbolStringFunctor {
            StringFormatContext sfc;
        public:
            WriteSymbolStringFunctor(const Context& context, const SymbolTable& symbols) : sfc{context, symbols} {
                sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::SymbolIds;
                sfc.format_info.hash_before_symbol_id = false;
                sfc.format_info.prefactor_join = StringFormatContext::PrefactorJoin::Asterix;
            }

            matlab::data::MATLABString operator()(const Polynomial& poly) const {
                return UTF8toUTF16Convertor::convert(poly.as_string(sfc));
            }

            matlab::data::MATLABString operator()(const Monomial& mono) const {
                return UTF8toUTF16Convertor::convert(mono.as_string(sfc));
            }
        };

        class WritePolyDataFunctor {
        public:
            matlab::data::ArrayFactory& factory;
            const  SymbolTable& symbol_table;
            PolynomialExporter polyExporter;

            WritePolyDataFunctor(
                    matlab::engine::MATLABEngine& engine,
                    matlab::data::ArrayFactory& factory,
                    const Context& context,
                    const SymbolTable& symbol_table,
                    const double zero_tolerance)
                : factory{factory}, symbol_table{symbol_table},
                   polyExporter{engine, factory, context, symbol_table, zero_tolerance} { }

            matlab::data::CellArray
            operator()(const Monomial& monomial) const {
                Polynomial asPoly{monomial};
                return (*this)(asPoly);
            }

            matlab::data::CellArray
            operator()(const Polynomial& polynomial) const {
                auto constituents = polyExporter.sequences(polynomial, true);
                return constituents.move_to_cell(factory);
            }
        };

        template<typename matrix_t>
        matlab::data::CellArray do_export_polynomials(const OperatorMatrixExporter& exporter,
                                                      const matrix_t& matrix) {

            auto output = exporter.factory.createCellArray(
                    OperatorMatrixExporter::matrix_dimensions(matrix)
            );

            auto readIter = matrix.SymbolMatrix().begin();
            const auto readIterEnd = matrix.SymbolMatrix().end();
            auto writeIter = output.begin();
            const auto writeIterEnd = output.end();

            exporter.do_write(readIter, readIterEnd, writeIter, writeIterEnd,
                              WritePolyDataFunctor{exporter.engine, exporter.factory,
                                                   exporter.context, exporter.symbol_table, exporter.zero_tolerance});

            return output;
        }

        template<typename matrix_t>
        matlab::data::StringArray do_export_symbol_strings(const OperatorMatrixExporter& exporter,
                                                           const matrix_t& matrix) {

            auto output = exporter.factory.createArray<matlab::data::MATLABString>(
                    OperatorMatrixExporter::matrix_dimensions(matrix)
            );

            auto readIter = matrix.SymbolMatrix().begin();
            const auto readIterEnd = matrix.SymbolMatrix().end();
            auto writeIter = output.begin();
            const auto writeIterEnd = output.end();

            exporter.do_write(readIter, readIterEnd, writeIter, writeIterEnd,
                              WriteSymbolStringFunctor{exporter.context, exporter.symbol_table});

            return output;
        }

        template<typename matrix_t>
        matlab::data::CellArray do_export_symbol_cell(const OperatorMatrixExporter& exporter, const matrix_t& matrix) {
            PolynomialExporter poly_exporter{exporter.engine, exporter.factory, exporter.context,
                                             exporter.symbol_table, exporter.zero_tolerance};

            return poly_exporter.symbol_cell_vector(matrix.SymbolMatrix(),
                                                    matlab::data::ArrayDimensions{matrix.Dimension(),
                                                                                  matrix.Dimension()});
        }
    }



    OperatorMatrixExporter::OperatorMatrixExporter(matlab::engine::MATLABEngine &engine, const MatrixSystem &system)
        : ExporterWithFactory{engine}, system{system}, context{system.Context()}, symbol_table{system.Symbols()},
          zero_tolerance{system.polynomial_factory().zero_tolerance},
          sequence_string_exporter{engine, factory, system} {
    }

    OperatorMatrixExporter::OperatorMatrixExporter(matlab::engine::MATLABEngine &engine,
                                                   const Locality::LocalityMatrixSystem &locality_system,
                                                   const Locality::LocalityOperatorFormatter &localityFormatter)
            : ExporterWithFactory{engine}, system{locality_system}, context{system.Context()}, symbol_table{system.Symbols()},
              zero_tolerance{system.polynomial_factory().zero_tolerance},
              sequence_string_exporter{engine, factory, locality_system, localityFormatter} {

    }

    void OperatorMatrixExporter::properties(IOArgumentRange &output, size_t matrix_index,
                                                   const SymbolicMatrix &theMatrix) const {
        switch (output.size()) {
            default:
            case 4:
                output[3] = factory.createScalar<bool>(theMatrix.Hermitian());
            case 3:
                output[2] = factory.createScalar<bool>(theMatrix.is_monomial());
            case 2:
                output[1] = factory.createScalar<uint64_t>(theMatrix.Dimension());
            case 1:
                output[0] = factory.createScalar<uint64_t>(matrix_index);
            case 0:
                break;
        }
    }

    FullMonomialSpecification OperatorMatrixExporter::monomials(const MonomialMatrix &matrix) const {
        FullMonomialSpecification output{this->factory, OperatorMatrixExporter::matrix_dimensions(matrix), true};

        if (!matrix.has_aliased_operator_matrix()) {
            throw InternalError{"Cannot convert matrix to monomials, if underlying operator sequences are not defined."};
        }

        auto read_iter = IterTuple{matrix.SymbolMatrix().begin(),
                                   matrix.aliased_operator_matrix()().begin()};

        const auto read_iter_end = IterTuple{matrix.SymbolMatrix().end(),
                                             matrix.aliased_operator_matrix()().end()};

        auto write_iter = output.full_write_begin();
        const auto write_iter_end = output.full_write_end();
        this->do_write(read_iter, read_iter_end, write_iter, write_iter_end,
                       FullMonomialSpecification::FullWriteFunctor{this->factory, this->symbol_table});

        return output;
    }

    matlab::data::StringArray OperatorMatrixExporter::name(const SymbolicMatrix &matrix) const {
        return this->factory.createScalar(matrix.Description());
    }

    matlab::data::CellArray OperatorMatrixExporter::symbol_cell(const SymbolicMatrix &matrix) const {
        if (matrix.is_monomial()) {
            return do_export_symbol_cell(*this, dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return do_export_symbol_cell(*this, dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::CellArray OperatorMatrixExporter::polynomials(const SymbolicMatrix &matrix) const {
        if (matrix.is_monomial()) {
            return do_export_polynomials(*this, dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return do_export_polynomials(*this, dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::StringArray OperatorMatrixExporter::sequence_strings(const SymbolicMatrix &matrix) const {
        if (matrix.is_monomial()) {
            return this->sequence_string_exporter(dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return this->sequence_string_exporter(dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::StringArray OperatorMatrixExporter::symbol_strings(const SymbolicMatrix &matrix) const {
        if (matrix.is_monomial()) {
            return do_export_symbol_strings(*this, dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return do_export_symbol_strings(*this, dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::ArrayDimensions OperatorMatrixExporter::matrix_dimensions(const SymbolicMatrix &matrix)  {
        return {matrix.Dimension(), matrix.Dimension()};
    }

}
