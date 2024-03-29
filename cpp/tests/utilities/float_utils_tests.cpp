/**
 * float_utils_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/float_utils.h"

namespace Moment::Tests {
    TEST(Utilities_FloatUtils, ApproximatelyEqual) {
        double x = 1.0;
        double y = 2.0;
        double z = 2.0;
        EXPECT_FALSE(approximately_equal(x, y));
        EXPECT_FALSE(approximately_equal(x, z));
        EXPECT_FALSE(approximately_equal(y, x));
        EXPECT_FALSE(approximately_equal(z, x));
        EXPECT_TRUE(approximately_equal(x, x));
        EXPECT_TRUE(approximately_equal(y, y));
        EXPECT_TRUE(approximately_equal(z, z));
        EXPECT_TRUE(approximately_equal(y, z));
    }

    TEST(Utilities_FloatUtils, EssentiallyEqual) {
        double x = 1.0;
        double y = 2.0;
        double z = 2.0;
        EXPECT_FALSE(essentially_equal(x, y));
        EXPECT_FALSE(essentially_equal(x, z));
        EXPECT_FALSE(essentially_equal(y, x));
        EXPECT_FALSE(essentially_equal(z, x));
        EXPECT_TRUE(essentially_equal(x, x));
        EXPECT_TRUE(essentially_equal(y, y));
        EXPECT_TRUE(essentially_equal(z, z));
        EXPECT_TRUE(essentially_equal(y, z));
    }

    TEST(Utilities_FloatUtils, ApproximatelyZero) {
        double x = 0.0;
        double y = 1.0;
        double z = 1e-20;
        EXPECT_TRUE(approximately_zero(x));
        EXPECT_FALSE(approximately_zero(y));
        EXPECT_TRUE(approximately_zero(z));
    }


    TEST(Utilities_FloatUtils, ApproximatelyReal) {
        std::complex<double> x = 0.0;
        std::complex<double> y = 1.0;
        std::complex<double> yi{0, 1.0};
        std::complex<double> z{1e-20, 0.0};
        std::complex<double> zi{1e-20, 0.0};
        EXPECT_TRUE(approximately_real(x));
        EXPECT_TRUE(approximately_real(y));
        EXPECT_TRUE(approximately_real(-y));
        EXPECT_FALSE(approximately_real(yi));
        EXPECT_FALSE(approximately_real(-yi));
        EXPECT_TRUE(approximately_real(z));
        EXPECT_TRUE(approximately_real(-z));
        EXPECT_TRUE(approximately_real(zi));
        EXPECT_TRUE(approximately_real(-zi));
    }

    TEST(Utilities_FloatUtils, ComplexApproximatelyZero) {
        std::complex<double> x = 0.0;
        std::complex<double> y = 1.0;
        std::complex<double> yi{0, 1.0};
        std::complex<double> z{1e-20, 0.0};
        std::complex<double> zi{1e-20, 0.0};
        EXPECT_TRUE(approximately_zero(x));
        EXPECT_FALSE(approximately_zero(y));
        EXPECT_FALSE(approximately_zero(-y));
        EXPECT_FALSE(approximately_zero(yi));
        EXPECT_FALSE(approximately_zero(-yi));
        EXPECT_TRUE(approximately_zero(z));
        EXPECT_TRUE(approximately_zero(-z));
        EXPECT_TRUE(approximately_zero(zi));
        EXPECT_TRUE(approximately_zero(-zi));
    }

    TEST(Utilities_FloatUtils, ComplexApproximatelyEqual) {
        using namespace std::complex_literals;

        std::complex<double> x = 1.0;
        std::complex<double> y = 2.0;
        std::complex<double> z = 2.0;
        std::complex<double> w{1.0, 1.0};
        EXPECT_FALSE(approximately_equal(x, y));
        EXPECT_FALSE(approximately_equal(x * 1i, y * 1i));
        EXPECT_FALSE(approximately_equal(x, y));
        EXPECT_FALSE(approximately_equal(x * 1i, z * 1i));
        EXPECT_FALSE(approximately_equal(x * 1i, z * 1i));
        EXPECT_FALSE(approximately_equal(x, z));
        EXPECT_FALSE(approximately_equal(y, x));
        EXPECT_FALSE(approximately_equal(z, x));
        EXPECT_FALSE(approximately_equal(y * 1i, z));
        EXPECT_TRUE(approximately_equal(x, x));
        EXPECT_TRUE(approximately_equal(x * 1i, x * 1i));
        EXPECT_TRUE(approximately_equal(y, y));
        EXPECT_TRUE(approximately_equal(z, z));
        EXPECT_TRUE(approximately_equal(y, z));
        EXPECT_TRUE(approximately_equal(w, w));
    }

    TEST(Utilities_FloatUtils, RealOrImaginaryIfClose) {
        using namespace std::complex_literals;

        std::complex<double> re{10, 1e-17};
        EXPECT_NE(re, std::complex(10.0, 0.0));
        real_or_imaginary_if_close(re);
        EXPECT_EQ(re, std::complex(10.0, 0.0));

        std::complex<double> minus_re{-10.0, 1e-17};
        EXPECT_NE(minus_re, std::complex(-10.0, 0.0));
        real_or_imaginary_if_close(minus_re);
        EXPECT_EQ(minus_re, std::complex(-10.0, 0.0));

        std::complex<double> im{1e-17, 10};
        EXPECT_NE(im, std::complex(0.0, 10.0));
        real_or_imaginary_if_close(im);
        EXPECT_EQ(im, std::complex(0.0, 10.0));

        std::complex<double> minus_im{1e-17, -10.0};
        EXPECT_NE(minus_im, std::complex(0.0, -10.0));
        real_or_imaginary_if_close(minus_im);
        EXPECT_EQ(minus_im, std::complex(0.0, -10.0));

        std::complex<double> complex{0.5, 0.5};
        real_or_imaginary_if_close(complex);
        EXPECT_EQ(complex, std::complex(0.5, 0.5));
    }

    TEST(Utilities_FloatUtils, ApproximatelyCompare) {
        const double x = 1.0;
        const double y = 1.0 + 1e-10;
        ASSERT_NE(x, y);

        EXPECT_EQ(approximately_compare(x, y, 1.0), -1);
        EXPECT_EQ(approximately_compare(y, x, 1.0), +1);
        EXPECT_EQ(approximately_compare(x, y, 10e5), 0);
    }
}