classdef (InferiorClasses={?Locality.Measurement, ?Locality.Outcome})...
          JointProbability < handle
%JOINTMEASUREMENT Collection of measurements / outcomes from different parties.
    properties(SetAccess=private, GetAccess=public)
        Scenario
        Marginals
        FixedOutcomes
        FreeIndices
        FixedIndices
    end
    
    properties(Dependent, GetAccess=public)
        ImplicitOutcomes
        Correlator
    end
    
    %% Private properties
    properties(Access=private)       
        implicit = MTKPolynomial.empty(1,0);
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
            arguments
                scenario (1,1) LocalityScenario
                free_measurements(1, :) Locality.Measurement
                fixed_outcomes(1, :) Locality.Outcome = Locality.Outcome.empty(1,0);
            end
            
            obj.Scenario = scenario;
            
            % Get marginal measurements
            [obj.Marginals, obj.FixedOutcomes] = ...
                obj.checkAndSortMmts(free_measurements, fixed_outcomes);
            
            [obj.FreeIndices, obj.FixedIndices] = ...
                obj.calculateIndices();
        end
    end
    
    %% Accessors
    methods      
        function val = get.ImplicitOutcomes(obj)
            if ~obj.Scenario.HasMatrixSystem
                error("Implicit outcomes cannot be generated before matrix system is created.");
            end

            if isempty(obj.implicit)
                poly_spec = mtk('probability_table', 'full_sequences',...
                    obj.Scenario.System.RefId, obj.FreeIndices, obj.FixedIndices);
                obj.implicit = ...
                    MTKPolynomial.InitFromOperatorCell(obj.Scenario, ...
                                                       poly_spec);
            end
            val = obj.implicit;            
        end
        
        function val = get.Correlator(obj)
            impl = obj.ImplicitOutcomes;
            if ~isequal(size(impl), [2 2])
                error("Correlator only defined between two binary measurements.");
            end
            val = impl(1,1) + impl(2,2) - impl(1,2) - impl(2,1);
        end
         
        function val = Outcome(obj, index)
            
%             arguments
%                 obj (1,1) Locality.JointMeasurement
%                 index (1, :) uint64
%             end
%             if length(index) ~= length(obj.Shape)
%                 error("Index should have " + length(obj.Shape) + " elements");
%             end
%             if any(index > obj.Shape)
%                 error("Index out of bounds.")
%             end
%             
%             indices = horzcat(obj.Indices, ...
%                               reshape(index, [length(obj.Shape), 1]));
%               
%             val = obj.Scenario.get(indices);
        end
        
        
        
        function val = ContainsParty(obj, party_index)
            val = any(obj.FreeIndices(:,1) == party_index) ...
                    || any(obj.FixedIndices(:,1) == party_index);
        end
                
        function val = ExplicitValues(obj, distribution)
            arguments
                obj Locality.JointMeasurement
                distribution (1,:) double
            end
            if length(distribution) ~= prod(obj.Shape)
                error("Distribution must match number of outcomes.");
            end
            val = mtk('make_explicit', obj.Scenario.System.RefId, ...
                      obj.Indices, distribution);
        end
        
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
    
     
    %% Convertors
    methods
        function poly = MTKPolynomial(obj)
        % MTKPOLYNOMIAL Cast to MTKPolynomial.
            poly = obj.ImplicitOutcomes;
        end
    end
    
    
    %% Private methods
    methods(Access=private)
        function [free, fixed] = checkAndSortMmts(obj, input_free, input_fixed)
                   
            % Get party indices from free inputs
            free_indices = zeros(1, numel(input_free));
            for i = 1:numel(input_free)
                if (input_free(i).Scenario ~= obj.Scenario)
                    error("Each measurement must be in the same scenario.");
                end
                free_indices(i) = input_free(i).Index(1);
            end
            
            % Get party indices from fixedinputs
            fixed_indices = zeros(1, numel(input_fixed));
            for i = 1:numel(input_fixed)
                if (input_fixed(i).Scenario ~= obj.Scenario)
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
            free = Locality.Measurement.empty(1,0);
            for i = 1:numel(input_free)
                free(end+1) = input_free(free_indices(i));
            end
            
            % Re-order fixed
            fixed = Locality.Outcome.empty(1,0);
            for i = 1:numel(input_fixed)
                fixed(end+1) = input_fixed(fixed_indices(i));
            end           
        end
        
        function [free, fixed] = calculateIndices(obj)
            free = uint64(zeros(numel(obj.Marginals), 2));
            for i = 1:numel(obj.Marginals)
                free(i,:) = obj.Marginals(i).Index(:);
            end
            
            fixed = uint64(zeros(numel(obj.FixedOutcomes), 3));
            for i = 1:numel(obj.FixedOutcomes)
                fixed(i,:) = obj.FixedOutcomes(i).Index(:);
            end            
        end           
    end
end

