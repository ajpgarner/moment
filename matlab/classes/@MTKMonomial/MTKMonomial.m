classdef MTKMonomial < MTKObject
%MTKMONOMIAL A monomial expression of operators (or their moment).
        
    %% Public properties
    properties(GetAccess = public, SetAccess = protected)
        Operators % The operator sequence defining this monomial.        
        Coefficient % Scalar co-efficient factor of the monomial.
        Hash % Hash of the operator sequence in this monomial.
    end
    
    %% Public dependent properties
    properties(Dependent, GetAccess = public)
        % True if monomial can be found in symbol table.
        FoundSymbol
        
        % The ID number of the corresponding symbol in table.
        SymbolId
        
        % True if this monomial represents a conjugation of the symbol in the table.
        SymbolConjugated
        
        % The real basis element representing the real part of this monomial, or 0.
        RealBasisIndex
        
        % The imaginary basis element representing the imaginary part of this monomial, or 0.
        ImaginaryBasisIndex
        
        % True, if monomial represents 0
        IsZero
    end
    
    %% Private properties
    properties(Access=private)
        symbol_id = int64(-1);
        symbol_conjugated = false;
        re_basis_index = -1;
        im_basis_index = -1;
    end
    
    %% Constructor
    methods
        function obj = MTKMonomial(setting, operators, scale)
        % MONOMIAL Construct a monomial.
        %
        % PARAMS
        %   setting - The algebraic scenario the monomial is a part of.
        %   operators - The operator string describing the monomial.
        %   scale - The scalar coefficient premultiplying the monomial.
        %
            array_creation = false;
            create_dimensions = [1, 1];
            init_for_overwrite = false;
            
            if (nargin < 1) || ~isa(setting, 'MTKScenario')
                error("First argument must be a scenario.");
            end
            
            if nargin < 2
                operators = uint64.empty(1,0); % ID
                scale = 1.0; 
            else
                if (ischar(operators) || isstring(operators)) ...
                        && strcmp(operators, 'overwrite')
                    init_for_overwrite = true;
                    if nargin < 3
                        error("Overwrite mode must supply dimensions as third argument.");
                    else
                        create_dimensions = double(scale);                   
                        array_creation = prod(create_dimensions) ~= 1;
                    end                    
                elseif isnumeric(operators)                   
                    operators = reshape(uint64(operators), 1, []);
                elseif iscell(operators)
                    array_creation = true;
                    create_dimensions = size(operators);
                else
                    error("Operators strings should be supplied as row vector, or cell array of row vectors.");
                end
                
                if ~init_for_overwrite
                    if nargin < 3
                        if array_creation
                            scale = ones(create_dimensions);
                        else
                            scale = 1.0;
                        end
                    else 
                        if array_creation
                            if length(scale) == 1
                                scale = ones(create_dimensions) * scale;
                            elseif ~isequal(size(operators), size(scale))
                                error("Scale dimensions must match operator specification dimensions.");
                            else
                                scale = double(scale);
                            end
                        else
                            if length(scale) ~= 1
                                error("Scale dimensions must match operator specification dimensions.");
                            end
                            scale = double(scale);
                        end
                    end
                end
            end

            % Superclass constructor
            obj = obj@MTKObject(setting, create_dimensions);
            
            if array_creation
                if init_for_overwrite
                    obj.Operators = cell(create_dimensions);
                    obj.Coefficient = zeros(create_dimensions);
                    obj.Hash = uint64(zeros(create_dimensions));                    
                else
                    [obj.Operators, negated, obj.Hash] = ...
                        setting.Simplify(operators);
                    neg_mask = ones(size(negated));
                    neg_mask(negated) = -1;
                    obj.Coefficient = scale .* neg_mask;
                    obj.Hash(obj.Coefficient == 0) = 0;                    
                end
            elseif ~init_for_overwrite
                 [obj.Operators, negated, obj.Hash] = ...
                     setting.Simplify(operators);
                 obj.Coefficient = scale;
                 if negated
                     obj.Coefficient = -obj.Coefficient;
                 end
                 if obj.Coefficient == 0
                     obj.Hash = 0;
                 end
            end
            
            obj.setDefaultSymbolInfo();
        end
    end
    
    %% Named constructors
    methods(Static)
        function obj = InitForOverwrite(setting, dimensions)
        % INITFOROVERWRITE Create blank monomial to overwrite.
            arguments
                setting (1,1) MTKScenario
                dimensions (1,:) double
            end
            obj = MTKMonomial(setting, 'overwrite', dimensions);
        end
        
        function obj = InitValue(setting, values)
        % INITVALUE Create monomials representing numeric values
            arguments
                setting (1,1) MTKScenario
                values (:,:) double
            end
            
            if numel(values) == 1
                obj = MTKMonomial(setting, [], values);
            else
                dims = num2cell(size(values));
                obj = MTKMonomial(setting, repelem({[]}, dims{:}), values);
            end
        end
    end
    
    %% Convertors
    methods
        function poly = MTKPolynomial(obj)
            if obj.IsScalar
                poly = MTKPolynomial(obj.Scenario, obj);
            else
                poly = MTKPolynomial(obj.Scenario, obj.split());
            end
        end
    end
    
    
    %% Accessors: Symbol row info
    methods
        function val = get.FoundSymbol(obj)
            if any(obj.symbol_id < 0)
                obj.loadSymbolInfo();
            end
            val = logical(obj.symbol_id >= 0);
        end
        
        function val = get.SymbolId(obj)
            obj.loadSymbolInfoOrError();
            val = obj.symbol_id;
        end
        
        function val = get.SymbolConjugated(obj)
            obj.loadSymbolInfoOrError();
            val = obj.symbol_conjugated;
        end
        
        function val = get.RealBasisIndex(obj)
            obj.loadSymbolInfoOrError();
            val = obj.re_basis_index;
        end
        
        function val = get.ImaginaryBasisIndex(obj)
            obj.loadSymbolInfoOrError();
            val = obj.im_basis_index;
        end
    end
    
    %% Accessors: Zero
    methods
        function val = get.IsZero(obj)
            if obj.IsScalar
                val = isempty(obj.Operators) && (obj.Coefficient == 0);
            else
                val = cellfun(@isempty, obj.Operators) ...
                        & (obj.Coefficient == 0);
            end
        end
    end
    
    %% Virtual methods (implemented).
    methods(Access=protected)
        
        [re, im] = calculateCoefficients(obj);
                        
        spliceOut(output, source, indices);
                
        [output, matched] = spliceProperty(obj, indices, propertyName);
                 
        mergeIn(obj, merge_dim, offsets, objects);
               
        str = makeObjectName(obj);        
    end
  
end
