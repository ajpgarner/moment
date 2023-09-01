/**
 * maintains_tensors.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "maintains_tensors.h"

#include "collins_gisin.h"
#include "full_correlator.h"
#include "probability_tensor.h"

namespace Moment  {

    MaintainsTensors::~MaintainsTensors() noexcept = default;

    const class CollinsGisin& MaintainsTensors::CollinsGisin() const {
        if (!this->collinsGisin) {
            throw Moment::errors::missing_component("Collins-Gisin tensor has not yet been generated. ");
        }
        return *this->collinsGisin;
    }

    const class FullCorrelator& MaintainsTensors::FullCorrelator() const {
        if (!this->fullCorrelator) {
            throw Moment::errors::missing_component("Full correlator tensor has not yet been generated. ");
        }
        return *this->fullCorrelator;
    }

    const class ProbabilityTensor& MaintainsTensors::ProbabilityTensor() const {
        if (!this->probabilityTensor) {
            throw Moment::errors::missing_component("Probability tensor has not yet been generated.");
        }
        return *this->probabilityTensor;
    }

    bool MaintainsTensors::RefreshCollinsGisin(std::shared_lock<std::shared_mutex>& read_lock) {
        // First, if no explicit symbol table at all, we surely need to do something
        if (!this->collinsGisin) {
            read_lock.unlock();

            auto write_lock = this->get_write_lock();
            this->collinsGisin = this->makeCollinsGisin();
            const bool has_all_symbols = this->collinsGisin->HasAllSymbols();
            write_lock.unlock();

            read_lock.lock();
            return has_all_symbols;
        }

        // No missing symbols, return without ever having released read lock
        if (this->collinsGisin->HasAllSymbols()) {
            return true;
        }

        // Upgrade lock
        read_lock.unlock();
        auto write_lock = this->get_write_lock();

        // Try to fill symbols
        const bool filled = this->collinsGisin->fill_missing_symbols();

        // Downgrade lock
        write_lock.unlock();
        read_lock.lock();
        return filled;
    }

    bool MaintainsTensors::RefreshCollinsGisin() {
        auto lock = this->get_read_lock();
        return this->RefreshCollinsGisin(lock);
    }


    bool MaintainsTensors::RefreshFullCorrelator(std::shared_lock<std::shared_mutex>& read_lock) {
        // First, ensure CG exists and is up-to-date.
        this->RefreshCollinsGisin(read_lock);

        // If no FC, create one if we can
        if (!this->fullCorrelator) {
            // Check validity
            if (!this->CanHaveFullCorrelator()) {
                throw std::logic_error{"Scenario does not support full correlator tensor."};
            }

            // Wait to upgrade locks...
            read_lock.unlock();
            auto write_lock = this->get_write_lock();

            if (!this->fullCorrelator) {  // Double-check, in case scooped.
                this->fullCorrelator = this->makeFullCorrelator();
            }

            const bool has_all_symbols = this->fullCorrelator->HasAllPolynomials();
            write_lock.unlock();

            read_lock.lock();
            return has_all_symbols;
        }

        // No missing symbols, return without ever having released read lock
        if (this->fullCorrelator->HasAllPolynomials()) {
            return true;
        }

        // Upgrade lock
        read_lock.unlock();
        auto write_lock = this->get_write_lock();

        // Try to fill symbols
        const bool filled = this->fullCorrelator->fill_missing_polynomials();

        // Downgrade lock
        write_lock.unlock();
        read_lock.lock();
        return filled;
    }


    bool MaintainsTensors::RefreshFullCorrelator() {
        auto lock = this->get_read_lock();
        return this->RefreshFullCorrelator(lock);
    }

    bool MaintainsTensors::RefreshProbabilityTensor(std::shared_lock<std::shared_mutex> &read_lock) {
        // First, ensure CG exists and is up-to-date.
        this->RefreshCollinsGisin(read_lock);

        // If no PT, create one
        if (!this->probabilityTensor) {
            // Wait to upgrade locks...
            read_lock.unlock();
            auto write_lock = this->get_write_lock();

            if (!this->probabilityTensor) {  // Double-check, in case scooped.
                this->probabilityTensor = this->makeProbabilityTensor();
            }
            const bool has_all_symbols = this->probabilityTensor->HasAllPolynomials();
            write_lock.unlock();

            read_lock.lock();
            return has_all_symbols;
        }

        // No missing symbols, return without ever having released read lock
        if (this->probabilityTensor->HasAllPolynomials()) {
            return true;
        }

        // Upgrade lock
        read_lock.unlock();
        auto write_lock = this->get_write_lock();

        // Try to fill symbols
        const bool filled = this->probabilityTensor->fill_missing_polynomials();

        // Downgrade lock
        write_lock.unlock();
        read_lock.lock();
        return filled;
    }

    bool MaintainsTensors::RefreshProbabilityTensor() {
        auto lock = this->get_read_lock();
        return this->RefreshProbabilityTensor(lock);
    }
}