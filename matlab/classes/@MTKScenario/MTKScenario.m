classdef (Abstract) MTKScenario < handle
% MTKSCENARIO A setting or context defining a set of operators.
% 
% A scenario amounts to a description of a system, with a set of operators,
% and the rules for multiplying and conjugating these operators, and for
% writing a string of operators in a 'canonical' manner.
%
% See also: AlgebraicScenario, ImportedScenario, InflationScenario, 
%           LocalityScenario
    
    
    %% Public configurable properties
    properties(Access = public)        
        % The multiplier of eps(1), below which we treate numbers as zero.
        ZeroTolerance;
        
        % True if individual operators are Hermitian
        IsHermitian;
        
        % True if operators are next to their Hermitian conjugates     
        Interleave;        
    end
    
    
    %% Public dependent properties
    properties(Dependent, GetAccess = public, SetAccess = private)
        % True if a matrix system has been created in mtk.
        HasMatrixSystem
        
        % Effective number of operators
        OperatorCount
        
        % Raw number of operators
        RawOperatorCount
        
        % Names of operators, indexed by operator number.
        OperatorNames
    end
    
    %% Private properties
    properties(Access = protected)
        matrix_system % A MatrixSystem handle for the given scenario.
        operator_names = string.empty(1, 0);  
    end
      
    %% Error messages
    properties(Constant, Access = private)
        % Error message when scenario is locked, but changes are requested.
        err_locked = [
            'This Scenario is locked, and no further changes are possible. ', ...
            'This is because it has been associated with a MatrixSystem ', ...
            '(e.g. at least one moment matrix has already been generated). ', ...
            'To make changes to this Scenario first create a deep copy using ', ...
            'scenario.Clone(), then make alterations to the copy.'];
        
        err_not_ready = ['Property "%s" is not available until a ', ...
                         'MatrixSystem has been created.'];
    end  
    
    %% Constructor (abstract base class)
    methods(Access = protected)
        function obj = MTKScenario(tolerance, is_hermitian, interleave)
        % SCENARIO Create an scenario object.
            arguments                
                tolerance (1,1) double
                is_hermitian (1,1) logical
                interleave (1,1) logical
            end
            
            obj.matrix_system = MatrixSystem.empty;
            obj.ZeroTolerance = tolerance;
            obj.IsHermitian = is_hermitian;
            obj.Interleave = interleave;
            
        end
    end
    
    %% Property setters
    methods
         function set.IsHermitian(obj, value)
             arguments
                obj (1,1) MTKScenario
                value (1,1) logical
            end
            obj.errorIfLocked();
            value = obj.onSetHermitian(obj.IsHermitian, value);
            obj.IsHermitian = value;
         end
         
         function set.Interleave(obj, value)
             arguments
                obj (1,1) MTKScenario
                value (1,1) logical
            end
            obj.errorIfLocked();
            obj.Interleave = value;
         end
         
         function set.ZeroTolerance(obj, value)
            arguments
                obj (1,1) MTKScenario
                value (1,1) double
            end
            obj.errorIfLocked();
            if value < 0
                error("ZeroTolerance must be non-negative.");
            end
            obj.ZeroTolerance = value;            
         end
    end
        
       
    %% Accessors
    methods
        function val = get.Interleave(obj)
            if obj.IsHermitian
                error("Interleave not defined when operators are Hermitian.");
            end
            val = obj.Interleave; 
        end
                
        function val = get.HasMatrixSystem(obj)
            val = ~isempty(obj.matrix_system);
        end
        
        function val = get.OperatorCount(obj)
            if isempty(obj.matrix_system)
                error(obj.err_not_ready, 'OperatorCount');
            end
            val = obj.operatorCount();
        end
        
        function val = get.RawOperatorCount(obj)
            if isempty(obj.matrix_system)
                error(obj.err_not_ready, 'RawOperatorCount');
            end
            val = obj.operatorCount();
            if ~obj.IsHermitian
                val = val * 2;
            end      
        end
        
        function val = get.OperatorNames(obj)            
            % Names are constantly refreshed until scenario is locked.
            if isempty(obj.operator_names) || isempty(obj.matrix_system)
                obj.operator_names = obj.makeOperatorNames();                
            end
            
            assert(~isempty(obj.operator_names));
            val = obj.operator_names;
        end            
    end
    
    %% Abstract virtual methods (must be overloaded)
    methods(Abstract, Access={?MTKScenario,?MatrixSystem})
        ref_id = createNewMatrixSystem(obj) 
    end
    
    methods(Abstract, Access=protected)
        val = operatorCount(obj);
    end
    
    %% Virtual methods (can be overloaded)
    methods(Access=protected)
        onNewMomentMatrix(obj, mm);
        val = createSolvedScenario(obj, a, b);
        str = makeOperatorNames(obj)
        val = onSetHermitian(obj, old_val, new_val);
    end
   
end
