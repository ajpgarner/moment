/**
 * export_operator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_operator_matrix.h"

#include "export_operator_sequence.h"
#include "export_polynomial.h"

#include "matrix_system.h"
#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "symbolic/polynomial_factory.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "utilities/utf_conversion.h"

#include "error_codes.h"
#include "utilities/reporting.h"



namespace Moment::mex {

    namespace {
        template<typename read_iter_t, typename write_iter_t, typename export_functor_t>
        void do_export(matlab::engine::MATLABEngine& engine, matlab::data::ArrayFactory& factory,
                       read_iter_t read_iter, const read_iter_t read_iter_end,
                       write_iter_t write_iter, const write_iter_t write_iter_end,
                       const export_functor_t& elem_writer) {


            while ((read_iter != read_iter_end) && (write_iter != write_iter_end)) {
                *write_iter = elem_writer(*read_iter);
                ++read_iter;
                ++write_iter;
            }

            // Sanity checks
            if (read_iter != read_iter_end) {
                throw_error(engine, errors::internal_error,
                            "Unexpectedly encountered end of write before end of read.");
            }
            if (write_iter != write_iter_end) {
                throw_error(engine, errors::internal_error,
                            "Unexpectedly encountered end of read before end of write.");
            };
        }

        template<typename matrix_t>
        matlab::data::StringArray do_export_symbol_strings(matlab::engine::MATLABEngine& engine,
                                                           matlab::data::ArrayFactory& factory,
                                                           const matrix_t& matrix) {

            auto output = factory.createArray<matlab::data::MATLABString>(
                OperatorMatrixExporter::matrix_dimensions(matrix)
            );

            auto readIter = matrix.SymbolMatrix().ColumnMajor.begin();
            const auto readIterEnd = matrix.SymbolMatrix().ColumnMajor.end();
            auto writeIter = output.begin();
            const auto writeIterEnd = output.end();

            do_export(engine, factory, readIter, readIterEnd, writeIter, writeIterEnd, WriteSymbolStringFunctor{});

            return output;
        }

        class WriteSymbolStringFunctor {
        public:
            matlab::data::MATLABString operator()(const Polynomial& poly) const {
                return UTF8toUTF16Convertor::convert(poly.as_string());
            }

            matlab::data::MATLABString operator()(const Monomial& mono) const {
                return UTF8toUTF16Convertor::convert(mono.as_string());
            }
        };

        class WriteMonoDataFunctor {
        public:
            matlab::data::ArrayFactory& factory;
            const  SymbolTable& symbol_table;

            explicit WriteMonoDataFunctor(matlab::data::ArrayFactory& factory, const SymbolTable& symbol_table)
                : factory{factory}, symbol_table{symbol_table} { }

            FullMonomialSpecification::full_iter_t::value_type
            operator()(std::tuple<const Monomial&, const OperatorSequence&> input) const {
                const auto& monomial = std::get<0>(input);
                const auto& op_seq = std::get<1>(input);
                assert(monomial.id >= 0 && monomial.id < this->symbol_table.size());
                const auto& symbol_info = this->symbol_table[monomial.id];

                return FullMonomialSpecification::full_iter_t::value_type{
                    export_operator_sequence(factory, op_seq, true), // ML indexing
                    monomial.factor,
                    op_seq.hash(),
                    monomial.id,
                    monomial.conjugated,
                    symbol_info.basis_key().first + 1, // ML indexing
                    symbol_info.basis_key().second + 1 // ML indexing
                };
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
                    const SymbolTable& symbol_table,
                    const double zero_tolerance)
                : factory{factory}, symbol_table{symbol_table}, polyExporter{engine, symbol_table, zero_tolerance} { }

            matlab::data::CellArray
            operator()(const Monomial& monomial) const {
                Polynomial asPoly{monomial};
                return (*this)(asPoly);
            }

            matlab::data::CellArray
            operator()(const Polynomial& polynomial) const {
                auto constituents = polyExporter.sequences(this->factory, polynomial, true);
                return constituents.move_to_cell(factory);
            }
        };


        template<typename matrix_t>
        matlab::data::CellArray do_export_polynomials(matlab::engine::MATLABEngine& engine,
                                                      matlab::data::ArrayFactory& factory,
                                                      const SymbolTable& symbol_table,
                                                      const double zero_tolerance,
                                                      const matrix_t& matrix) {

            auto output = factory.createCellArray(
                    OperatorMatrixExporter::matrix_dimensions(matrix)
            );

            auto readIter = matrix.SymbolMatrix().ColumnMajor.begin();
            const auto readIterEnd = matrix.SymbolMatrix().ColumnMajor.end();
            auto writeIter = output.begin();
            const auto writeIterEnd = output.end();

            do_export(engine, factory, readIter, readIterEnd, writeIter, writeIterEnd,
                      WritePolyDataFunctor{engine, factory, symbol_table, zero_tolerance});

            return output;
        }
    }

    OperatorMatrixExporter::OperatorMatrixExporter(matlab::engine::MATLABEngine &engine, const MatrixSystem &system)
        : Exporter{engine}, system{system}, context{system.Context()}, symbol_table{system.Symbols()},
          sequence_string_exporter{engine, system} {
    }

    OperatorMatrixExporter::OperatorMatrixExporter(matlab::engine::MATLABEngine &engine,
                                                   const Locality::LocalityMatrixSystem &locality_system,
                                                   const Locality::LocalityOperatorFormatter &localityFormatter)
            : Exporter{engine}, system{locality_system}, context{system.Context()}, symbol_table{system.Symbols()},
              sequence_string_exporter{engine, locality_system, localityFormatter} {

    }

    FullMonomialSpecification OperatorMatrixExporter::monomials(const MonomialMatrix &matrix) const {
        FullMonomialSpecification output{this->factory, OperatorMatrixExporter::matrix_dimensions(matrix), true};

        if (!matrix.has_operator_matrix()) {
            throw_error(this->engine, errors::internal_error,
                        "Cannot convert matrix to monomials, if underlying operator sequences are not defined.");
        }

        auto read_iter = IterTuple{matrix.SymbolMatrix().ColumnMajor.begin(),
                                   matrix.operator_matrix()().ColumnMajor.begin()};

        const auto read_iter_end = IterTuple{matrix.SymbolMatrix().ColumnMajor.end(),
                                             matrix.operator_matrix()().ColumnMajor.end()};

        auto write_iter = output.full_write_begin();
        const auto write_iter_end = output.full_write_end();
        do_export(this->engine, this->factory,
                  read_iter, read_iter_end, write_iter, write_iter_end,
                  WriteMonoDataFunctor{this->factory, this->symbol_table});

        return output;
    }

    matlab::data::StringArray OperatorMatrixExporter::name(const Matrix &matrix) const {
        return this->factory.createScalar(matrix.Description());
    }

    matlab::data::CellArray OperatorMatrixExporter::polynomials(const Matrix &matrix) const {
        if (matrix.is_monomial()) {
            return do_export_polynomials(this->engine, this->factory, this->symbol_table,
                                         this->system.polynomial_factory().zero_tolerance,
                                         dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return do_export_polynomials(this->engine, this->factory, this->symbol_table,
                                         this->system.polynomial_factory().zero_tolerance,
                                         dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::StringArray OperatorMatrixExporter::sequence_strings(const Matrix &matrix) const {
        if (matrix.is_monomial()) {
            return this->sequence_string_exporter(dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return this->sequence_string_exporter(dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::StringArray OperatorMatrixExporter::symbol_strings(const Matrix &matrix) const {
        if (matrix.is_monomial()) {
            return do_export_symbol_strings(this->engine, this->factory,
                                            dynamic_cast<const MonomialMatrix&>(matrix));
        } else {
            return do_export_symbol_strings(this->engine, this->factory,
                                            dynamic_cast<const PolynomialMatrix&>(matrix));
        }
    }

    matlab::data::ArrayDimensions OperatorMatrixExporter::matrix_dimensions(const Matrix &matrix)  {
        return {matrix.Dimension(), matrix.Dimension()};
    }

}
