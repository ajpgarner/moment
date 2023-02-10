classdef Scenario < handle
    %SCENARIO 
    
    properties(GetAccess = public, SetAccess = protected)
        HasMatrixSystem
    end
    
    properties(Access = protected)
        matrix_system
    end
      
    properties(Constant, Access = protected)
        err_locked = [
            'This Scenario is locked, and no further changes are possible. ', ...
            'This is because it has been associated with a MatrixSystem ', ...
            '(e.g. at least one moment matrix has already been generated). ', ...
            'To make changes to this Scenario first create a deep copy using ', ...
            'scenario.Clone(), then make alterations to the copy.'];
    end  
    
    %% Constructor (abstract base class)
    methods(Access = protected)
        function obj = Scenario()
            obj.matrix_system = MatrixSystem.empty;
        end
    end
       
    %% Accessors: MatrixSystem
    methods
        function val = System(obj)
            arguments
                obj (1,1) Abstract.Scenario
            end
            
            % Make matrix system, if not already generated
            if isempty(obj.matrix_system)
                obj.matrix_system = MatrixSystem(obj);
            end
            
            val = obj.matrix_system;
        end       
        
        
        function val = get.HasMatrixSystem(obj)
            val = ~isempty(obj.matrix_system);
        end
    end
    
    %% Accessor: SolvedScenario
    methods
        function val = Solved(obj, a, b)
            if nargin < 2
                a = double.empty(1, 0);
            end
            if nargin < 3
                b = double.empty(1, 0);
            end
            
            val = obj.createSolvedScenario(a, b);            
        end
    end
    
    %% Accessors: SymbolTable
    methods
        function val = Symbols(obj)
            val = struct2table(obj.System.SymbolTable);
        end
    end
    
    %% Accessors: Operator matrices
    methods
         function mm_out = MakeMomentMatrix(obj, depth)
            arguments
                obj (1,1) Abstract.Scenario
                depth (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            
            % Construct matrix
            mm_out = OpMatrix.MomentMatrix(obj.System(), depth);
            
            % Do binding...
            obj.onNewMomentMatrix(mm_out)
        end
    end
            
    %% Internal/friend methods
    methods(Access={?Abstract.Scenario,?Locality.Party})
        function errorIfLocked(obj)
            if ~isempty(obj.matrix_system)
                error(obj.err_locked);
            end
        end
    end
    
    %% Abstract methods
    methods(Abstract, Access=protected)
        % Post-process on new moment matrix
        onNewMomentMatrix(obj, mm)        
    end
    
    %% Friend/interface methods
    methods(Abstract, Access={?Abstract.Scenario,?MatrixSystem})
        % Query mex for a matrix system
        ref_id = createNewMatrixSystem(obj)
    end
    
    %% Virtual methods 
    methods(Access=protected)
        function val = createSolvedScenario(obj, a, b)
            val = SolvedScenario.SolvedScenario(obj, a, b);
        end
    end
        
    
    
end

