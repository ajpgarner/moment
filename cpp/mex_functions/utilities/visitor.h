/**
 * visitor.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * Template implements the "visitor" idiom, allowing for dynamic (i.e. runtime) dispatch of a MATLAB array to a functor
 * equipped to handle the appropriate array type. Functors should implement at least one of the visitor concepts defined
 * in this file.
 */
#pragma once

#include "errors.h"

#include <concepts>
#include <stdexcept>

#include "MatlabDataArray.hpp"
#include "mex.hpp"

namespace Moment::mex {

    namespace concepts {

        /**
         * True if functor can process real, dense, numeric arrays.
         * @tparam functor_t The visited functor class that will be applied to the array.
         */
        template <class functor_t>
        concept VisitorHasRealDense = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;

            {functor.template dense<int8_t>(matlab::data::TypedArray<int8_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int16_t>(matlab::data::TypedArray<int16_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int32_t>(matlab::data::TypedArray<int32_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int64_t>(matlab::data::TypedArray<int64_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int8_t>(matlab::data::TypedArray<uint8_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int16_t>(matlab::data::TypedArray<uint16_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int32_t>(matlab::data::TypedArray<uint32_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<int64_t>(matlab::data::TypedArray<uint64_t>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<float>(matlab::data::TypedArray<float>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<double>(matlab::data::TypedArray<double>{a})}
                -> std::same_as<typename functor_t::return_type>;
        };

        /**
         * True if functor can process complex, dense, numeric arrays.
         * @tparam functor_t The visited functor class that will be applied to the array.
         */
        template <class functor_t>
        concept VisitorHasComplexDenseFloat = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.template dense<std::complex<float>>(matlab::data::TypedArray<std::complex<float>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<double>>(matlab::data::TypedArray<std::complex<double>>{a})}
                -> std::same_as<typename functor_t::return_type>;
        };

        /**
         * True if functor can process complex, dense, integral arrays.
         * @tparam functor_t The visited functor class that will be applied to the array.
         */
        template <class functor_t>
        concept VisitorHasComplexDenseInteger = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.template dense<std::complex<int8_t>>(matlab::data::TypedArray<std::complex<int8_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<int16_t>>(matlab::data::TypedArray<std::complex<int16_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<int32_t>>(matlab::data::TypedArray<std::complex<int32_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<int64_t>>(matlab::data::TypedArray<std::complex<int64_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<uint8_t>>(matlab::data::TypedArray<std::complex<uint8_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<uint16_t>>(matlab::data::TypedArray<std::complex<uint16_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<uint32_t>>(matlab::data::TypedArray<std::complex<uint32_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
            {functor.template dense<std::complex<uint64_t>>(matlab::data::TypedArray<std::complex<uint64_t>>{a})}
                -> std::same_as<typename functor_t::return_type>;
        };

        /**
         * True if functor can process logical/boolean, dense arrays.
         * @tparam functor_t The visited functor class that will be applied to the array.
         */
        template <class functor_t>
        concept VisitorHasBooleanDense = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.template dense<bool>(static_cast<matlab::data::TypedArray<bool>>(a))} -> std::same_as<typename functor_t::return_type>;
        };

        /**
         * True if functor can process real, sparse, numeric arrays.
         * @tparam functor_t The visited functor class that will be applied to the array.
         */
        template <class functor_t>
        concept VisitorHasRealSparse = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.template sparse<double>(static_cast<matlab::data::SparseArray<double>>(a))} -> std::same_as<typename functor_t::return_type>;
        };

        /**
          * True if functor can process complex, sparse, numeric arrays.
          * @tparam functor_t The visited functor class that will be applied to the array.
          */
        template <class functor_t>
        concept VisitorHasComplexSparse = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.template sparse<std::complex<double>>(static_cast<matlab::data::SparseArray<std::complex<double>>>(a))} -> std::same_as<typename functor_t::return_type>;
        };

        /**
          * True if functor can process logical/boolean, sparse arrays.
          * @tparam functor_t The visited functor class that will be applied to the array.
          */
        template <class functor_t>
        concept VisitorHasBooleanSparse = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.template sparse<bool>(static_cast<matlab::data::SparseArray<bool>>(a))} -> std::same_as<typename functor_t::return_type>;
        };

        /**
         * True if functor can process arrays of matlab strings.
         * @tparam functor_t The visited functor class that will be applied to the array.
         */
        template <class functor_t>
        concept VisitorHasString = requires(functor_t& functor, matlab::data::Array& a) {
            typename functor_t::return_type;
            {functor.string(static_cast<matlab::data::StringArray>(a))} -> std::same_as<typename functor_t::return_type>;
        };
    }

    /**
     * Dispatch class, bound to a functor.
     * @tparam functor_t The function to apply to the supplied array inputs.
     */
    template <class functor_t>
    class VisitDispatcher {
    protected:
        matlab::engine::MATLABEngine &engine;
        functor_t& visitor;

    public:
        static constexpr bool has_real_dense = concepts::VisitorHasRealDense<functor_t>;
        static constexpr bool has_complex_dense_float = concepts::VisitorHasComplexDenseFloat<functor_t>;
        static constexpr bool has_complex_dense_int = concepts::VisitorHasComplexDenseInteger<functor_t>;
        static constexpr bool has_boolean_dense = concepts::VisitorHasBooleanDense<functor_t>;
        static constexpr bool has_real_sparse = concepts::VisitorHasRealSparse<functor_t>;
        static constexpr bool has_complex_sparse = concepts::VisitorHasComplexSparse<functor_t>;
        static constexpr bool has_boolean_sparse = concepts::VisitorHasBooleanSparse<functor_t>;
        static constexpr bool has_string = concepts::VisitorHasString<functor_t>;

        using return_t = typename functor_t::return_type;

    public:
        /**
         * Construct a class allowing for the application of appropriate methods in a functor to a MATLAB array.
         * @param the_engine The MATLAB engine instance.
         * @param the_visitor Instance of the functor class.
         */
        explicit VisitDispatcher(matlab::engine::MATLABEngine &the_engine,
                                 functor_t& the_visitor) : engine(the_engine), visitor(the_visitor) { }


    private:
        template<typename matrix_t>
        return_t invoke(matrix_t data, typename std::enable_if<
                            std::is_base_of<matlab::data::Array,
                                            typename std::remove_reference<matrix_t>::type>::value,
                            void>::type* /*unused*/= nullptr) {
            const auto& as_array = static_cast<const matlab::data::Array&>(data);

            // Dense numeric real types
            if constexpr (has_real_dense) {
                switch (as_array.getType()) {
                    case matlab::data::ArrayType::INT8:
                    case matlab::data::ArrayType::INT16:
                    case matlab::data::ArrayType::INT32:
                    case matlab::data::ArrayType::INT64:
                    case matlab::data::ArrayType::UINT8:
                    case matlab::data::ArrayType::UINT16:
                    case matlab::data::ArrayType::UINT32:
                    case matlab::data::ArrayType::UINT64:
                    case matlab::data::ArrayType::SINGLE:
                    case matlab::data::ArrayType::DOUBLE:
                        return do_real_dense_invocation(std::forward<matrix_t>(data));
                    default:
                        break;
                }
            }

            // Dense numeric complex types
            if constexpr(has_complex_dense_int) {
                switch (as_array.getType()) {
                    case matlab::data::ArrayType::COMPLEX_INT8:
                    case matlab::data::ArrayType::COMPLEX_INT16:
                    case matlab::data::ArrayType::COMPLEX_INT32:
                    case matlab::data::ArrayType::COMPLEX_INT64:
                    case matlab::data::ArrayType::COMPLEX_UINT8:
                    case matlab::data::ArrayType::COMPLEX_UINT16:
                    case matlab::data::ArrayType::COMPLEX_UINT32:
                    case matlab::data::ArrayType::COMPLEX_UINT64:
                        return do_complex_dense_int_invocation(std::forward<matrix_t>(data));
                    default:
                        break;
                }
            }

            // Dense numeric complex types
            if constexpr(has_complex_dense_float) {
                switch (as_array.getType()) {
                    case matlab::data::ArrayType::COMPLEX_SINGLE:
                    case matlab::data::ArrayType::COMPLEX_DOUBLE:
                        return do_complex_dense_float_invocation(std::forward<matrix_t>(data));
                    default:
                        break;
                }
            }

            // Dense logical type
            if constexpr(has_boolean_dense) {
                if (as_array.getType() == matlab::data::ArrayType::LOGICAL) {
                    return do_boolean_dense_invocation(std::forward<matrix_t>(data));
                }
            }

            // Sparse types:
            if constexpr (has_real_sparse) {
                if (as_array.getType() == matlab::data::ArrayType::SPARSE_DOUBLE) {
                    return do_real_sparse_invocation(std::forward<matrix_t>(data));
                }
            }
            if constexpr (has_complex_sparse) {
                if (as_array.getType() == matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE) {
                    return do_complex_sparse_invocation(std::forward<matrix_t>(data));
                }
            }
            if constexpr (has_boolean_sparse) {
                if (as_array.getType() == matlab::data::ArrayType::SPARSE_LOGICAL) {
                    return do_boolean_sparse_invocation(std::forward<matrix_t>(data));
                }
            }

            if constexpr (has_string) {
                switch (as_array.getType()) {
                    case matlab::data::ArrayType::CHAR:
                    case matlab::data::ArrayType::MATLAB_STRING:
                        return do_string_invocation(std::forward<matrix_t>(data));
                    default:
                        break;
                }
            }

            throw BadVisitorException{"Unexpected type."};
        }

        template<typename matrix_t>
        inline return_t do_real_dense_invocation(matrix_t data) {

            const auto& as_array = static_cast<const matlab::data::Array&>(data);
            switch (as_array.getType()) {
                case matlab::data::ArrayType::INT8:
                    return this->visitor.template dense<int8_t>(matlab::data::TypedArray<int8_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::INT16:
                    return this->visitor.template dense<int16_t>(matlab::data::TypedArray<int16_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::INT32:
                    return this->visitor.template dense<int32_t>(matlab::data::TypedArray<int32_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::INT64:
                    return this->visitor.template dense<int64_t>(matlab::data::TypedArray<int64_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::UINT8:
                    return this->visitor.template dense<uint8_t>(matlab::data::TypedArray<uint8_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::UINT16:
                    return this->visitor.template dense<uint16_t>(matlab::data::TypedArray<uint16_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::UINT32:
                    return this->visitor.template dense<uint32_t>(matlab::data::TypedArray<uint32_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::UINT64:
                    return this->visitor.template dense<uint64_t>(matlab::data::TypedArray<uint64_t>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::SINGLE:
                    return this->visitor.template dense<float>(matlab::data::TypedArray<float>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::DOUBLE:
                    return this->visitor.template dense<double>(matlab::data::TypedArray<double>{std::forward<matrix_t>(data)});
                default:
                    break;
            }
            throw BadVisitorException{"Unexpected array type (real, dense)."};
        }

        template<typename matrix_t>
        inline return_t do_complex_dense_int_invocation(matrix_t data) {
            const auto& as_array = static_cast<const matlab::data::Array&>(data);
            switch (as_array.getType()) {
                case matlab::data::ArrayType::COMPLEX_INT8:
                    return this->visitor.template dense<std::complex<int8_t>>(matlab::data::TypedArray<std::complex<int8_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_INT16:
                    return this->visitor.template dense<std::complex<int16_t>>(matlab::data::TypedArray<std::complex<int16_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_INT32:
                    return this->visitor.template dense<std::complex<int32_t>>(matlab::data::TypedArray<std::complex<int32_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_INT64:
                    return this->visitor.template dense<std::complex<int64_t>>(matlab::data::TypedArray<std::complex<int64_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_UINT8:
                    return this->visitor.template dense<std::complex<uint8_t>>(matlab::data::TypedArray<std::complex<uint8_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_UINT16:
                    return this->visitor.template dense<std::complex<uint16_t>>(matlab::data::TypedArray<std::complex<uint16_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_UINT32:
                    return this->visitor.template dense<std::complex<uint32_t>>(matlab::data::TypedArray<std::complex<uint32_t>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_UINT64:
                    return this->visitor.template dense<std::complex<uint64_t>>(matlab::data::TypedArray<std::complex<uint64_t>>{std::forward<matrix_t>(data)});
            }
        }

        template<typename matrix_t>
        inline return_t do_complex_dense_float_invocation(matrix_t data) {
            const auto& as_array = static_cast<const matlab::data::Array&>(data);
            switch (as_array.getType()) {
                case matlab::data::ArrayType::COMPLEX_SINGLE:
                    return this->visitor.template dense<std::complex<float>>(matlab::data::TypedArray<std::complex<float>>{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::COMPLEX_DOUBLE:
                    return this->visitor.template dense<std::complex<double>>(matlab::data::TypedArray<std::complex<double>>{std::forward<matrix_t>(data)});
                default:
                    break;
            }
            throw BadVisitorException{"Unexpected array type (complex, dense)."};
        }

        template<typename matrix_t>
        inline return_t do_boolean_dense_invocation(matrix_t data) {
            return this->visitor.template dense<bool>(matlab::data::TypedArray<bool>{std::forward<matrix_t>(data)});
        }

        template<typename matrix_t>
        inline return_t do_real_sparse_invocation(matrix_t data) {
            return this->visitor.template sparse<double>(matlab::data::SparseArray<double>{std::forward<matrix_t>(data)});
        }

        template<typename matrix_t>
        inline return_t do_complex_sparse_invocation(matrix_t data) {
            return this->visitor.template sparse<std::complex<double>>(matlab::data::SparseArray<std::complex<double>>{std::forward<matrix_t>(data)});
        }

        template<typename matrix_t>
        inline return_t do_boolean_sparse_invocation(matrix_t data) {
            return this->visitor.template sparse<bool>(matlab::data::SparseArray<bool>{std::forward<matrix_t>(data)});
        }

        template<typename matrix_t>
        inline return_t do_string_invocation(matrix_t data) {
            const auto& as_array = static_cast<const matlab::data::Array&>(data);
            switch (as_array.getType()) {
                case matlab::data::ArrayType::MATLAB_STRING:
                    return this->visitor.string(matlab::data::StringArray{std::forward<matrix_t>(data)});
                case matlab::data::ArrayType::CHAR:
                    return this->visitor.string(matlab::data::CharArray{std::forward<matrix_t>(data)});
                default:
                    break;
            }
            throw BadVisitorException{"Unexpected array type (string)."};
        }


    public:
        /**
         * Invokes the functor on supplied MATLAB array.
         * @param data rvalue reference input MATLAB array.
         * @return Object of functor_t::return_type, as a result of the functor's application.
         */
        inline return_t operator()(matlab::data::Array&& data) {
            return invoke<matlab::data::Array&&>(std::move(data));
        }

        /**
         * Invokes the functor on supplied MATLAB array.
         * @param data Reference to input MATLAB array.
         * @return Object of functor_t::return_type, as a result of the functor's application.
         */
        inline return_t operator()(matlab::data::Array& data) {
            return invoke<matlab::data::Array&>(data);
        }

        /**
         * Invokes the functor on supplied MATLAB array.
         * @param data Read-only reference to input MATLAB array.
         * @return Object of functor_t::return_type, as a result of the functor's application.
         */
        inline return_t operator()(const matlab::data::Array& data) {
            return invoke<const matlab::data::Array&>(data);
        }
    };

    /**
     * Calls the appropriate function within a functor on a MATLAB array/matrix.
     * @tparam functor_t The functor class that will be applied to the input matrix.
     * @tparam matrix_t A MATLAB array type to be passed into the function.
     * @param engine The MATLAB engine.
     * @param matrix The input matrix to act upon.
     * @param visitor Instance of the functor that will act on the input matrix.
     * @return Object of functor_t::return_type, as a result of the function.
     */
    template <class functor_t, typename matrix_t>
    auto DispatchVisitor(matlab::engine::MATLABEngine& engine, matrix_t matrix, functor_t&& visitor) {
        VisitDispatcher dispatcher{engine, visitor};
        return dispatcher(std::forward<matrix_t>(matrix));
    }


}