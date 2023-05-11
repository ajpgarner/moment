classdef Scenario < handle
%SCENARIO A setting or context defining a set of operators.
% 
% A scenario amounts to a description of a system, with a set of operators,
% and the rules for multiplying and conjugating these operators, and for
% writing a string of operators in a 'canonical' manner.
%
% See also: AlgebraicScenario, ImportedScenario, InflationScenario, 
%           LocalityScenario
    
    properties(GetAccess = public, SetAccess = protected)
        HasMatrixSystem % True if a matrix system has been created in mtk.
    end
    
    properties(Access = protected)
        matrix_system % A MatrixSystem handle for the given scenario.
    end
      
    properties(Constant, Access = private)
        % Error message when scenario is locked, but changes are requested.
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
        % SCENARIO Create an scenario object.
            obj.matrix_system = MatrixSystem.empty;
        end
    end
       
    %% Accessors: MatrixSystem
    methods
        function val = System(obj)
        % SYSTEM Gets MatrixSystem object associated with scenario
        %
        % Will generate the MatrixSystem if it has not yet been
        % created.
        %
        % RETURN:
        %   A MatrixSystem object.
        %
        % See also: MatrixSystem
        %
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
    methods(Sealed)
        function val = Solved(obj, a, b)
        % SOLVED Bind numeric values from an SDP solve.
        % 
        % PARAMS:
        %   a - Vector of coefficients for real basis elements.
        %   b (optional) - Vector of coefficients for imaginary basis
        %                  elements.
        %
        % RETURNS:
        %   SOLVEDSCENARIO.SOLVEDSCENARIO or specialized subclass thereof.
        %
        % See also: SOLVEDSCENARIO.SOLVEDSCENARIO
        %
            arguments
                obj (1,1) Abstract.Scenario
                a (:,1) double = double.empty(0,1)
                b (:,1) double = double.empty(0,1)
            end
            
            if nargin <= 2
        	    b = double.empty(0,1);
        	end
            
            val = obj.createSolvedScenario(a, b);            
        end
    end
    
    %% Accessors: SymbolTable
    methods(Sealed)
        function val = Symbols(obj)
        % SYMBOLS Get table of defined symbols in the matrix system.
        %
        % RETURNS:
        %   Table object with symbol information. 
            arguments
                obj (1,1) Abstract.Scenario
            end
     
            val = struct2table(obj.System.SymbolTable);
        end
    end
    
  
    %% Accessors: Operator matrices
    methods(Sealed)
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
         
         function output = Simplify(obj, input)
         % SIMPLIFY Get canonical form of operator sequence input.
             arguments
                 obj(1,1) Abstract.Scenario
                 input(1,:)
             end
             
             output = mtk('simplify', obj.System.RefId, input);
         end
    end
            
    %% Internal/friend methods
    methods(Sealed, Access={?Abstract.Scenario,?Locality.Party})
        function errorIfLocked(obj)
        % ERRORIFLOCKED Raise an error if matrix system is already created.
            if ~isempty(obj.matrix_system)
                error(obj.err_locked);
            end
        end
    end
    
    %% Abstract methods
    methods(Abstract, Access=protected)
        % ONNEWMOMENTMATRIX Post-processing after moment matrix created.
        onNewMomentMatrix(obj, mm)
       
    end
    
    %% Friend/interface methods
    methods(Abstract, Access={?Abstract.Scenario,?MatrixSystem})   
        ref_id = createNewMatrixSystem(obj) 
    end
    
    %% Virtual methods 
    methods(Access=protected)
        function val = createSolvedScenario(obj, a, b)
        % CREATESOLVEDSCENARIO Bind numeric values from an SDP solve.
        % 
        % PARAMS
        %   a - Vector of values for real basis elements
        %   b (optional) - Vector of values for imaginary basis elements
        %
        % RETURNS
        %   SOLVEDSCENARIO.SOLVEDSCENARIO or specialized subclass thereof.
        %
            val = SolvedScenario.SolvedScenario(obj, a, b); 
        end
    end
end
