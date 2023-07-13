/**
 * maintains_tensors.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_system.h"
#include <memory>

namespace Moment {

    class CollinsGisin;
    class ProbabilityTensor;

    /**
     * Mix-in for MatrixSystem, to maintain Collins-Gisin tensor and Probability-Tensor
     */
    class MaintainsTensors : public MatrixSystem {

    protected:
        template<typename... Args>
        explicit MaintainsTensors(Args&&... args) : MatrixSystem(std::forward<Args>(args)...) { }

    public:
        virtual ~MaintainsTensors() noexcept;

    protected:
        std::unique_ptr<class CollinsGisin> collinsGisin;

        std::unique_ptr<class ProbabilityTensor> probabilityTensor;

    public:
        /**
         * Returns an indexing in the Collins-Gisin ordering.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class CollinsGisin& CollinsGisin() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements).
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class ProbabilityTensor& ProbabilityTensor() const;

        /**
         * Checks if it is necessary to refresh the explicit Collins-Gisin table, and refresh it if so.
         * If a refresh is necessary msReadLock will be released, and system will wait for write lock. Read-lock will be
         * reacquired after write is complete.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshCollinsGisin(std::shared_lock<std::shared_mutex>& read_lock);

        /**
         * Checks if it is necessary to refresh the explicit symbol table, and refresh it if so.
         * Acquires write-lock if refresh is necessary: either release locks before calling, or use the overload
         * with a read-lock parameter.
         * @return
         */
        bool RefreshCollinsGisin();

        /**
         * Checks if it is necessary to refresh the implicit probability symbol table, and refresh it if so.
         * If a refresh is necessary msReadLock will be released, and system will wait for write lock. Read-lock will be
         * reacquired after write is complete.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshProbabilityTensor(std::shared_lock<std::shared_mutex>& read_lock);

        /**
         * Checks if it is necessary to refresh the implicit probability symbol table, and refresh it if so.
         * Acquires write-lock if refresh is necessary: either release locks before calling, or use the overload
         * with a read-lock parameter.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshProbabilityTensor();

    private:
        virtual std::unique_ptr<class CollinsGisin> makeCollinsGisin() = 0;

        virtual std::unique_ptr<class ProbabilityTensor> makeProbabilityTensor() = 0;


    };
}