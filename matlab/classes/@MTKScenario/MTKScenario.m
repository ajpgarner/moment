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
        ZeroTolerance = 100;
                
        % True if individual operators are Hermitian
        IsHermitian = true;
        
        % True if operators are next to their Hermitian conjugates     
        Interleave = true;
    end
    
    % True if the scenario defines operators
    properties(GetAccess = public, SetAccess = protected)
        % If true, scenario defines a set of operators.
        DefinesOperators = true;
        
        % If true, distinct operator sequences can map to same symbol.        
        PermitsSymbolAliases = false;
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
        matrix_system % A MTKMatrixSystem handle for the given scenario.
        operator_names = string.empty(1, 0);  
    end
      
    %% Error messages
    properties(Constant, Access = private)
        % Error message when scenario is locked, but changes are requested.
        err_locked = [
            'This Scenario is locked, and no further changes are possible. ', ...
            'This is because it has been associated with a matrix system ', ...
            '(e.g. at least one moment matrix has already been generated). ', ...
            'To make changes to this Scenario first create a deep copy using ', ...
            'scenario.Clone(), then make alterations to the copy.'];
        
        err_not_ready = ['Property "%s" is not available until a ', ...
                         'MatrixSystem has been created.'];
                     
        err_no_ops = "%s does not define any operators.";
    end  
    
    %% Constructor (abstract base class)
    methods(Access = protected)
        function obj = MTKScenario(varargin)
        % SCENARIO Create an scenario object.
            
            % Parse options
            options = MTKUtil.check_varargin_keys(...
                        MTKScenario.ctorArgNames(), varargin);
            defines_operators = true;
            zero_tolerance = 100;
            is_hermitian = true;
            interleave = true;
            implies_operators = false;
            
            for idx = 1:2:nargin
                switch options{idx}
                    case 'defines_operators'
                        defines_operators = logical(options{idx+1});
                    case 'tolerance'
                        zero_tolerance = double(options{idx+1});
                    case 'hermitian'
                        is_hermitian = logical(options{idx+1});
                        implies_operators = true;
                    case 'interleave'
                        interleave = logical(options{idx+1});                        
                        implies_operators = true;
                end
            end
            
            if implies_operators && ~defines_operators
                error("Cannot set 'hermitian' or 'interleave' flags,"...
                    + " when no operators are defined.");
            end
        
            
            obj.matrix_system = MTKMatrixSystem.empty;
            obj.ZeroTolerance = zero_tolerance;       
            if defines_operators
                obj.DefinesOperators = true;
                obj.IsHermitian = is_hermitian;
                obj.Interleave = interleave;
            else
                obj.DefinesOperators = false;
            end
                        
        end
    end
    
    %% Help class
    methods(Static, Access = protected)
        function val = ctorArgNames()
        % CTORARGNAMES List of named parameters for the constructor.
            val = ["tolerance", "hermitian", ...
                   "interleave", "defines_operators"];
        end
    end
    
    %% Property setters
    methods
         function set.IsHermitian(obj, value)             
            obj.errorIfNoOperators();
            obj.errorIfLocked();
            assert(isscalar(value));
                        
            value = obj.onSetHermitian(obj.IsHermitian, logical(value));
            obj.IsHermitian = value;
         end
         
         function set.Interleave(obj, value)             
            obj.errorIfNoOperators();            
            obj.errorIfLocked();
            assert(isscalar(value));
            
            obj.Interleave = logical(value);
         end
         
         function set.ZeroTolerance(obj, value)
            obj.errorIfLocked();
            assert(isscalar(value));
            if value < 0
                error("ZeroTolerance must be non-negative.");
            end
            obj.ZeroTolerance = double(value); 
         end
    end
        
       
    %% Accessors
    methods
        function val = get.Interleave(obj)
            obj.errorIfNoOperators();
            
            if obj.IsHermitian
                error("Interleave not defined when operators are Hermitian.");
            end
            val = obj.Interleave; 
        end
                
        function val = get.HasMatrixSystem(obj)
            val = ~isempty(obj.matrix_system);
        end
        
        function val = get.OperatorCount(obj)
            obj.errorIfNoOperators();
            
            if isempty(obj.matrix_system)
                error(obj.err_not_ready, 'OperatorCount');
            end
            val = obj.operatorCount();
        end
        
        function val = get.RawOperatorCount(obj)
            obj.errorIfNoOperators();
            
            if isempty(obj.matrix_system)
                error(obj.err_not_ready, 'RawOperatorCount');
            end
            val = obj.operatorCount();
            if ~obj.IsHermitian
                val = val * 2;
            end      
        end
        
        function val = get.OperatorNames(obj)      
            obj.errorIfNoOperators();
            
            % Names are constantly refreshed until scenario is locked.
            if isempty(obj.operator_names) || isempty(obj.matrix_system)
                obj.operator_names = obj.makeOperatorNames();                
            end
            
            assert(~isempty(obj.operator_names));
            val = obj.operator_names;
        end            
    end
    
    %% Abstract virtual methods (must be overloaded)
    methods(Abstract, Access={?MTKScenario,?MTKMatrixSystem})
        ref_id = createNewMatrixSystem(obj) 
    end
    
    methods(Abstract, Access=protected)
        val = operatorCount(obj);
    end
    
    %% Virtual methods (can be overloaded)
    methods(Access=protected)
        str = makeOperatorNames(obj)
        val = onSetHermitian(obj, old_val, new_val);
    end
   
end
