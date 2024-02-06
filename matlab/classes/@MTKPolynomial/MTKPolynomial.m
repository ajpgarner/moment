classdef (InferiorClasses={?MTKMonomial}) MTKPolynomial < MTKObject
%MTKPOLYNOMIAL A polynomial expression of operators (or their moments).
    
    %% Public properties
    properties(GetAccess=public, SetAccess=protected)
        Constituents = MTKMonomial.empty(1,0)
    end
        
    %% Public dependent properties
    properties(Dependent, GetAccess=public, SetAccess=private)
        IsZero
        OperatorCell
        SymbolCell
        
        % True if symbols for each element can be found in symbol table.
        FoundSymbol
        
        % True if all symbols for every element can be found in symbol table.
        FoundAllSymbols
    end
    
    %% Cached properties
    properties(Access=private)
        symbol_cell = cell(0, 0);        
        operator_cell = cell(0, 0);
        done_sc = false;
        done_oc = false;
    end
    
    %% Error strings
    properties(Constant,Access=private)
        err_bad_ctr_input = ['Second argument must be either a Monomial',...
                              ' array, or a cell array of Monomials'];
                          
        err_missing_symbol = ['Not all constituent symbols were ', ...
                              'identified in the MatrixSystem.'];
    end

    %% Constructor
    methods
        function obj = MTKPolynomial(setting, varargin)
        % POLYNOMIAL Constructs an algebraic polynomial object
        %
        % Syntax:
        %  1.   p = MTKPolynomial(setting, [vector of monomials]
        %  2.   p = MTKPolynomial(setting, {Cell of [vector of monomials]}
        %  3.   p = MTKPolynomial(setting, 'overwrite', [creation size])
        %
        % Syntax 1 creates a scalar polynomial.
        % Syntax 2 creates an array of polynomials.
        % Syntax 3 creates an uninitialized (scalar/array of) polynomial(s).
        %
        
            % Check argument 1
            if nargin < 1
                error("Scenario must be provided.");
            elseif ~isa(setting, 'MTKScenario')
                error("First argument must be a scenario.");
            end
            
            % Check if argument 3 is set
            if nargin >= 3 && isequal(varargin{2}, 'direct')
                no_checks = true;
            else
                no_checks = false;
            end

            % Check argument 2
            if nargin < 2
                create_dimensions = [1, 1];
                array_construction = false;
                zero_construction = true;
                constituents = MTKMonomial.empty(1,0);
            else
                if isa(varargin{1}, 'MTKMonomial')
                    create_dimensions = [1, 1];
                    array_construction = false;
                    constituents = varargin{1};
                    zero_construction = isempty(constituents);
                elseif isa(varargin{1}, 'cell')
                    constituents = interpretCellInput(setting, varargin{1});                    
                    create_dimensions = size(constituents);
                    if numel(constituents) == 1
                        constituents = constituents{1};
                        create_dimensions = [1, 1];
                        array_construction = false;                        
                    else
                        array_construction = true;
                    end
                    zero_construction = false;
                elseif (isstring(varargin{1}) || ischar(varargin{1})) ...
                        && strcmp(varargin{1}, 'overwrite')
                    create_dimensions = double(varargin{2});
                    array_construction = prod(create_dimensions) ~= 1;
                    zero_construction = true;                    
                else
                    error(MTKPolynomial.err_bad_ctr_input);
                end
            end

            % Parent c'tor
            obj = obj@MTKObject(setting, create_dimensions);
            
            % Quick construction for empty object
            if zero_construction
                if array_construction
                    cell_dims = num2cell(create_dimensions);
                    obj.Constituents = ...
                        repelem({MTKMonomial.empty(0,1)}, cell_dims{:});                     
                else
                    obj.Constituents = MTKMonomial.empty(0,1);
                end
                return;
            end
            
            if no_checks
                obj.Constituents = constituents;                
            else
                if ~array_construction
                    obj.Constituents = obj.orderAndMerge(constituents);
                else
                    obj.Constituents = cell(size(constituents));
                    for idx = 1:numel(constituents)
                        obj.Constituents{idx} = obj.orderAndMerge(constituents{idx});
                    end                
                end
            end
        end    
    end
    
    
    %% Named constructor
    methods(Static)
        function obj = InitForOverwrite(setting, dimensions)
        % INITFOROVERWRITE Create a blank polynomial object
        %
        % See also: SYMBOLIC.POLYNOMIAL.POLYNOMIAL
        %
            obj = MTKPolynomial(setting, 'overwrite', dimensions);
        end
        
        function obj = InitZero(setting, dimensions)
            obj = MTKPolynomial(setting, 'overwrite', dimensions);
        end
        
        obj = InitValue(setting, values);
        
        obj = InitFromOperatorPolySpec(setting, cell_input);
        
        obj = InitFromOperatorCell(setting, cell_input);
    end
    
    %% Convertors
    methods
        function mono = MTKMonomial(obj)
            error("Down-conversion not yet implemented.");
        end
        
        function val = MTKSymbolicObject(obj)
            val = MTKSymbolicObject(obj.Scenario, obj.SymbolCell, 'raw');
        end
    end
    
    %% Dependent variables
    methods
        function val = get.IsZero(obj)
            if obj.IsScalar
                val = isempty(obj.Constituents);
            else
                val = cellfun(@(x) isempty(x), obj.Constituents);
            end
        end
        
        function val = get.OperatorCell(obj)
            if ~obj.done_oc
                obj.makeOperatorCell();
                if ~obj.done_oc
                    error("Could not make operator cell.");
                end
            end
            val = obj.operator_cell;
        end
        
        function val = get.SymbolCell(obj)
            if ~obj.done_sc
                if ~obj.FoundAllSymbols
                    error("Cannot make symbol cell before symbols are found.");
                end
                obj.makeSymbolCell();
                if ~obj.done_sc
                    error("Could not make symbol cell.");
                end
            end
            val = obj.symbol_cell;
        end
        
        function val = get.FoundSymbol(obj)
            val = checkSymbolsFound(obj);
        end
        
        function val = get.FoundAllSymbols(obj)
            val = checkAllSymbolsFound(obj);
        end
    end
    
    %% Virtual method implementations
    methods(Access=protected)
        [re, im] = calculateCoefficients(obj);
        
        [mask_re, mask_im, elems_re, elems_im] = queryForMasks(obj);
        
        mode = spliceIn(obj, indices, value);
        
        spliceOut(output, source, indices);
                
        [output, matched] = spliceProperty(obj, indices, propertyName);
          
        mergeIn(obj, merge_dim, offsets, objects);
        
        str = makeObjectName(obj);         
        
        result = isPropertyMTKObject(obj, property_name);
    end
end

