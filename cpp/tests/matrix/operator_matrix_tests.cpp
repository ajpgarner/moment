/**
 * operator_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "compare_os_matrix.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

namespace Moment::Tests {


    class Matrix_OperatorMatrix : public ::testing::Test {
    private:
        std::unique_ptr<Pauli::PauliMatrixSystem> ms_ptr;
        const Pauli::PauliContext& context;

    protected:
        //symbol_name_t s_a, s_b, s_c;
        OperatorSequence I, iI, mI, miI,
                         x, ix, mx, mix,
                         y, iy, my, miy,
                         z, iz, mz, miz;

        symbol_name_t sI, sX, sY, sZ;

    public:
        Matrix_OperatorMatrix()
            : ::testing::Test{},
                ms_ptr{std::make_unique<Pauli::PauliMatrixSystem>(std::make_unique<Pauli::PauliContext>(1))},
                context{ms_ptr->pauliContext},
                I{OperatorSequence{context}},
                iI{OperatorSequence{{}, context, SequenceSignType::Imaginary}},
                mI{OperatorSequence{{}, context, SequenceSignType::Negative}},
                miI{OperatorSequence{{}, context, SequenceSignType::NegativeImaginary}},
                x{context.sigmaX(0)},
                ix{context.sigmaX(0, SequenceSignType::Imaginary)},
                mx{context.sigmaX(0, SequenceSignType::Negative)},
                mix{context.sigmaX(0, SequenceSignType::NegativeImaginary)},
                y{context.sigmaY(0)},
                iy{context.sigmaY(0, SequenceSignType::Imaginary)},
                my{context.sigmaY(0, SequenceSignType::Negative)},
                miy{context.sigmaY(0, SequenceSignType::NegativeImaginary)},
                z{context.sigmaZ(0)},
                iz{context.sigmaZ(0, SequenceSignType::Imaginary)},
                mz{context.sigmaZ(0, SequenceSignType::Negative)},
                miz{context.sigmaZ(0, SequenceSignType::NegativeImaginary)}
            {

                auto& mm1 = this->ms_ptr->MomentMatrix(1);
                auto& symbols = ms_ptr->Symbols();
                this->sI = 1;
                this->sX = symbols.where(x)->Id();
                this->sY = symbols.where(y)->Id();
                this->sZ = symbols.where(z)->Id();
        }

    protected:
        void SetUp() override {

        }

        [[nodiscard]] Pauli::PauliMatrixSystem& get_system() noexcept { return *this->ms_ptr; }

        [[nodiscard]] const Pauli::PauliContext& get_context() noexcept {
            return this->ms_ptr->pauliContext;
        }

        [[nodiscard]] const SymbolTable &get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory &get_factory() noexcept {
            return this->ms_ptr->polynomial_factory();
        }
    };



    TEST_F(Matrix_OperatorMatrix, PreMultiply_Single) {
        auto& pms = this->get_system();

        // Make and check MM
        auto& mmRaw = pms.MomentMatrix(1);
        compare_os_matrix(mmRaw, 4, {I, x, y, z,
                                        x, I, iz, miy,
                                        y, miz, I, ix,
                                        z, iy, mix, I});


        ASSERT_TRUE(mmRaw.has_operator_matrix());
        auto& mm_ops = mmRaw.operator_matrix();
        ASSERT_EQ(mm_ops.Dimension(), 4);
        auto zMMptr = mm_ops.pre_multiply(z);
        const auto& zMM = *zMMptr;
        compare_os_matrix("Z*MM", zMM, 4, {z, iy, mix, I,
                                           iy, z, iI, mx,
                                           mix, miI, z, my,
                                           I, x, y, z});

    }

    TEST_F(Matrix_OperatorMatrix, PostMultiply_Single) {
        auto& pms = this->get_system();

        // Make and check MM
        auto& mm = pms.MomentMatrix(1);
        compare_os_matrix(mm, 4, {I, x, y, z,
                                     x, I, iz, miy,
                                     y, miz, I, ix,
                                     z, iy, mix, I});

        ASSERT_TRUE(mm.has_operator_matrix());
        auto& mm_ops = mm.operator_matrix();
        ASSERT_EQ(mm_ops.Dimension(), 4);

        auto MMzptr = mm_ops.post_multiply(z);
        const auto& MMz = *MMzptr;
        compare_os_matrix("MM * z", MMz, 4, {z, miy, ix , I,
                                             miy, z, iI, x,
                                             ix, miI, z, y,
                                             I, mx, my, z});

    }

    TEST_F(Matrix_OperatorMatrix, PreMultiply_Polynomial) {
        auto& pms = this->get_system();

        // Make and check MM
        auto& mmRaw = pms.MomentMatrix(1);
        compare_os_matrix(mmRaw, 4, {I, x, y, z,
                                        x, I, iz, miy,
                                        y, miz, I, ix,
                                        z, iy, mix, I});


        ASSERT_TRUE(mmRaw.has_operator_matrix());
        auto& mm_ops = mmRaw.operator_matrix();
        ASSERT_EQ(mm_ops.Dimension(), 4);

        // Make and check polynomial
        Polynomial poly{Polynomial::storage_t{Monomial{sX, 1.0}, Monomial{sZ, 1.0}}};
        ASSERT_EQ(poly.size(), 2);

        // Do pre-multiply
        auto x_pluszMMptr = mm_ops.pre_multiply(poly, this->get_symbols());
        ASSERT_EQ(x_pluszMMptr.size(), 2);

        const auto& xMM = *x_pluszMMptr[0];
        compare_os_matrix("X*MM", xMM, 4, {x, I, iz, miy,
                                           I, x, y, z,
                                           iz, my, x, iI,
                                           miy, mz, miI, x});

        const auto& zMM = *x_pluszMMptr[1];
        compare_os_matrix("Z*MM", zMM, 4, {z, iy, mix, I,
                                           iy, z, iI, mx,
                                           mix, miI, z, my,
                                           I, x, y, z});

    }

    TEST_F(Matrix_OperatorMatrix, PostMultiply_Polynomial) {
        auto& pms = this->get_system();

        // Make and check MM
        auto& mmRaw = pms.MomentMatrix(1);
        compare_os_matrix(mmRaw, 4, {I, x, y, z,
                                     x, I, iz, miy,
                                     y, miz, I, ix,
                                     z, iy, mix, I});


        ASSERT_TRUE(mmRaw.has_operator_matrix());
        auto& mm_ops = mmRaw.operator_matrix();
        ASSERT_EQ(mm_ops.Dimension(), 4);

        // Make and check polynomial
        Polynomial poly{Polynomial::storage_t{Monomial{sX, 1.0}, Monomial{sZ, 1.0}}};
        ASSERT_EQ(poly.size(), 2);

        // Do pre-multiply
        auto x_pluszMMptr = mm_ops.post_multiply(poly, this->get_symbols());
        ASSERT_EQ(x_pluszMMptr.size(), 2);

        const auto& MMx = *x_pluszMMptr[0];
        compare_os_matrix("MM * x", MMx, 4, {x, I, miz, iy,
                                           I, x, my, mz,
                                           miz, y, x, iI,
                                           iy, z, miI, x});

        const auto& MMz = *x_pluszMMptr[1];
        compare_os_matrix("MM * z", MMz, 4, {z, miy, ix , I,
                                             miy, z, iI, x,
                                             ix, miI, z, y,
                                             I, mx, my, z});


    }

}
