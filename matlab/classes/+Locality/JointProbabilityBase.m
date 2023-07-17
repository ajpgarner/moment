classdef JointProbabilityBase < handle
%JOINTMEASUREMENTBASE Collection of measurements / outcomes from different parties.
%
% See also: LOCALITY.JOINTPROBABILITY, INFLATION.JOINTPROBABILITY
%

    properties(SetAccess=private, GetAccess=public)
        Scenario
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
    methods(Access=protected)
        function obj = JointProbabilityBase(scenario, free_indices, fixed_indices)
        % JOINTMEASUREMENT Construct a joint measurement.
        %
        % SYNTAX:
        %   JointMeasurement(scenario, [mmtA, mmtB])
        %   JointMeasurement(scenario, [mmtA, mmtB], [fixedC])       
        %
        
            obj.Scenario = scenario;                        
            obj.FreeIndices = free_indices;
            obj.FixedIndices = fixed_indices;
        end
    end
    
     
    %% Convertors
    methods
        function poly = MTKPolynomial(obj)
        % MTKPOLYNOMIAL Cast to MTKPolynomial.
            poly = obj.ImplicitOutcomes;
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
                obj.implicit.ReadOnly = true;
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

                
        function val = Apply(obj, re_vals, ~)
        % APPLY Forward to Apply function of explicit values.
            impl = obj.ImplicitOutcomes;
            val = impl.Apply(re_vals);
            
        end
    end
    
    
    %% Probability handling / rule making
    methods                       
        function val = Probability(obj, distribution, varargin)
        % PROBABILITY Create rules imposing probability distribution.
            val = Locality.make_explicit(obj.Scenario, ...
                obj.FreeIndices, obj.FixedIndices, false, ...
                distribution, varargin{:});           
                
        end
        
        function val = ConditionalProbability(obj, distribution, varargin)
        % CONDITIONALPROBABILITY Create rules imposing conditional probability distribution.
            val = Locality.make_explicit(obj.Scenario, ...
                obj.FreeIndices, obj.FixedIndices, true, ...
                distribution, varargin{:});      
        end        
    end    
end

