classdef (InferiorClasses={?Locality.Measurement, ?Locality.Outcome})...
          JointProbability < Locality.JointProbabilityBase
%JOINTMEASUREMENT Collection of measurements / outcomes from different parties.
    properties(SetAccess=private, GetAccess=public)
        Marginals
        FixedOutcomes
    end
  
    %% Constructor
    methods
        function obj = JointProbability(scenario, free_measurements, fixed_outcomes)
        % JOINTMEASUREMENT Construct a joint measurement.
        %
        % SYNTAX:
        %   JointMeasurement(scenario, [mmtA, mmtB])
        %   JointMeasurement(scenario, [mmtA, mmtB], [fixedC])
        %
        %
            
            %  Validate inputs
            if nargin < 1 || ~isa(scenario, 'LocalityScenario')
                error("First argument must be a locality scenario.");
            end
                        
            if nargin < 2 || isempty(free_measurements)
                free_measurements = Locality.Measurement.empty(1,0);
            elseif ~isa(free_measurements, 'Locality.Measurement')
                error("Second argument must be locality scenario measurements.");
            end
            
            if nargin < 3 || isempty(fixed_outcomes)
                fixed_outcomes = Locality.Outcome.empty(1,0);
            elseif ~isa(fixed_outcomes, 'Locality.Outcome')
                error("Third argument must be locality scenario outcomes.");
            end
            
            % Sort, and get indices
            [free_measurements, fixed_outcomes] = ...
                Locality.JointProbability.checkAndSortMmts(scenario, ...
                                 free_measurements, fixed_outcomes);
            
            [free_indices, fixed_indices] = ...
                Locality.JointProbability.calculateIndices(...
                    free_measurements, fixed_outcomes);
            
            % Construct object
            obj = obj@Locality.JointProbabilityBase(scenario, ...
                                                    free_indices, ...
                                                    fixed_indices);
            obj.Marginals = free_measurements;
            obj.FixedOutcomes = fixed_outcomes;           
        end
    end
    
    %% Accessors
    methods                             
        function val = ContainsParty(obj, party_index)
            val = any(obj.FreeIndices(:,1) == party_index) ...
                    || any(obj.FixedIndices(:,1) == party_index);
        end        
    end
    
    
    %% Algebraic manipulation
    methods
        function val = mtimes(lhs, rhs)
        % MTIMES Whole-object multiplication.
            
            if ~isa(lhs, 'Locality.JointProbability')
                this = rhs;
                other = lhs;
                this_on_left = false;
            else
                this = lhs;
                other = rhs;
                this_on_left = true;
            end
            
            % If numeric RHS, cast to polynomial and return:
            if isnumeric(other)
                impl = this.ImplicitOutcomes;
                if this_on_left 
                    val = mtimes(impl, other);
                else
                    val = mtimes(other, impl);
                end
                return
            end
            
            % Compose measurements
            if isa(other, 'Locality.Measurement')
                val = Locality.JointProbability(this.Scenario, ...
                    [this.Marginals, other], this.FixedOutcomes);                
                return;
            end
            
            if isa(other, 'Locality.Outcome')
                val = Locality.JointProbability(this.Scenario, ...
                    this.Marginals, [this.FixedOutcomes, other]);
                return;
            end
            
            if isa(other, 'Locality.JointProbability')
                val = Locality.JointProbability(this.Scenario, ...
                    [this.Marginals, other.Marginals], ...
                    [this.FixedOutcomes, other.FixedOutcomes]);
                return
            end
            
            % Complain
            error("Multiplication not defined between %s and %s", ...
                  class(lhs), class(rhs));            
        end
        
        function val = times(lhs, rhs)
        % TIMES Element-wise multiplication .*
            if ~isa(lhs, 'Locality.JointProbability')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % If numeric RHS, cast to polynomial and return:
            if isnumeric(other)
                impl = this.ImplicitOutcomes;
                val = times(impl, other);
                return
            end
            
            % Complain
            error("Element-wise multiplication not defined between %s and %s", ...
                  class(lhs), class(rhs));
            
        end
    end
    
    %% Private methods
    methods(Static, Access=private)
        function [free, fixed] = checkAndSortMmts(scenario, ...
                                                  input_free, input_fixed)
                   
            % Get party indices from free inputs
            free_indices = zeros(1, numel(input_free));
            for i = 1:numel(input_free)
                if (input_free(i).Scenario ~= scenario)
                    error("Each measurement must be in the same scenario.");
                end
                free_indices(i) = input_free(i).Index(1);
            end
            
            % Get party indices from fixed inputs
            fixed_indices = zeros(1, numel(input_fixed));
            for i = 1:numel(input_fixed)
                if (input_fixed(i).Scenario ~= scenario)
                    error("Each measurement must be in the same scenario.");
                end
                fixed_indices(i) = input_fixed(i).Index(1);
            end
            
            % Check for duplicates
            all_indices = [free_indices, fixed_indices];
            if numel(all_indices ) ~= length(unique(all_indices))
                error("Each measurement must be from a different party.");
            end
            
            % Re-order free
            [~, sort_free] = sort(free_indices);
            free = Locality.Measurement.empty(1,0);
            for i = 1:numel(input_free)
                free(end+1) = input_free(sort_free(i));
            end
            
            % Re-order fixed
            [~, sort_fixed] = sort(fixed_indices);
            fixed = Locality.Outcome.empty(1,0);
            for i = 1:numel(input_fixed)
                fixed(end+1) = input_fixed(sort_fixed(i));
            end           
        end
        
        function [free, fixed] = calculateIndices(free_measurements, ...
                                                  fixed_outcomes)
                                              
            free = uint64(zeros(numel(free_measurements), 2));
            for i = 1:numel(free_measurements)
                free(i,:) = free_measurements(i).Index(:);
            end
            
            fixed = uint64(zeros(numel(fixed_outcomes), 3));
            for i = 1:numel(fixed_outcomes)
                fixed(i,:) = fixed_outcomes(i).Index(:);
            end            
        end           
    end
end

