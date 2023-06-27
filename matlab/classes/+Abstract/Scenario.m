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
    
    properties(Access = public)
        ZeroTolerance;
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
        function obj = Scenario(tolerance)
        % SCENARIO Create an scenario object.
        arguments
            tolerance (1,1) double = 100
        end        
            obj.matrix_system = MatrixSystem.empty;
            obj.ZeroTolerance = tolerance;
        end
    end
    
    %% ZeroTolerance and rounding
    methods
         function set.ZeroTolerance(obj, value)
            arguments
                obj (1,1) Abstract.Scenario
                value (1,1) double
            end
            obj.errorIfLocked();
            if value < 0
                error("ZeroTolerance must be non-negative.");
            end
            obj.ZeroTolerance = value;            
         end
         
         function [val, mask, real_mask, im_mask] = Prune(obj, val, scale)
         % PRUNE Remove terms close to zero.
             arguments
                 obj (1,1)  Abstract.Scenario
                 val (:,:)  double
                 scale (1,1) double = 1.0;
             end
             
             if nargin < 3
                 scale = 1.0;
             end
             
             % Zero, if close
             mask = abs(val) < (obj.ZeroTolerance * eps(scale));
             val(mask) = 0.0;
             
             % Real, if close
             real_mask = abs(imag(val)) < (obj.ZeroTolerance * eps(scale));
             val(real_mask) = real(val(real_mask));
             
             % Imaginary, if close
             im_mask = abs(real(val)) < (obj.ZeroTolerance * eps(scale));
             val(im_mask) = 1i * imag(val(im_mask));             
         end
         
         function output = IsClose(obj, lhs, rhs, scale)
              arguments
                 obj (1,1)  Abstract.Scenario
                 lhs double,
                 rhs double
                 scale (1,1) double = 1.0;
             end
             if nargin < 4
                 scale = 1.0;
             end
             assert(isequal(size(lhs), size(rhs)));
             if ~isreal(lhs) || ~isreal(rhs)
                 diff = (lhs - rhs);
                 mod_diff = diff .* conj(diff);
                 output = mod_diff < (obj.ZeroTolerance ...
                                      * obj.ZeroTolerance ...
                                      * eps(scale) * eps(scale));             
             else
                 output = abs(lhs - rhs) < obj.ZeroTolerance * eps(scale);
             end
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
    end
    
    %% Operator sequence manipulations
    methods         
         function [output, negated, hash] = Simplify(obj, input)
         % SIMPLIFY Get canonical form of operator sequence input.
         % All applicable re-write rules will be applied.
         %
         % SYNTAX
         %      1. [output, negated, hash] = setting.Simplify(input_str)
         %      2. [o., n., h.] = setting.Simplify({cell of input_strs})
         %
         % The hash is the shortlex hash associated with this context.
         % In syntax 1, output is a uint64 row-vector, negated is scalar 
         % bool and hash is scalar uint64.
         % In syntax 2, output will be a cell array of uint64 row-vectors,
         % and negated and hash will be arrays of matching dimensions.
         %
             arguments
                 obj(1,1) Abstract.Scenario
                 input
             end
             
             [output, negated, hash] = mtk('simplify', obj.System.RefId, input);
         end
         
         function [output, negated, hash] = Conjugate(obj, input)
         % CONJUGATE Get canonical form of operator sequence's conjugate.
         %
         % SYNTAX
         %      1. output = setting.Simplify(input_str)
         %      2. [output, negated, hash] = setting.Simplify(input_str)
         %
         % The optional hash is the shortlex hash associated with this 
         % context.
         %
             arguments
                 obj(1,1) Abstract.Scenario
                 input
             end
                          
             [output, negated, hash] = ...
                 mtk('conjugate', obj.System.RefId, input);
         end
         
         function output = WordList(obj, length, register)
         % WORDLIST Get all operator sequences up to requested length.
         %
         % PARAMS
         %  length - The maximum length monomial to generate
         %  register - Set true to register generated monomials as symbols.
         %
             arguments
                 obj(1,1) Abstract.Scenario
                 length(1,1) uint64
                 register(1,1) logical = false
             end

             extra_args = cell(1,0);
             if register
                 extra_args{end+1} = 'register_symbols';
             end

             output = mtk('word_list', obj.System.RefId, length, ...
                          extra_args{:});

             if register
                 obj.System.UpdateSymbolTable();
             end
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
