 classdef ComplexObject < handle & matlab.mixin.CustomDisplay
% COMPLEXOBJECT An object that can be evaluated to a complex number.
%
% A complex object is defined by its real and imaginary coefficients,
% corresponding to weightings to give to the real and imaginary basis
% elements in a scenario's symbol table.
%
% For solution column vectors 'a' and 'b', the value of a complex object is 
% given by "a . real_coefs + b . im_coefs".
%
    
    properties(GetAccess = public, SetAccess = private)
        % Associated scenario handle
        Scenario
    end
    
    properties(Dependent, GetAccess = public, SetAccess = private)
        % Human-readable description of object.
        ObjectName
        
        % Real coefficients as a complex row vector.
        RealCoefficients      
        
        % Imaginary coefficients as a complex row vector.
        ImaginaryCoefficients 
        
    end
        
    properties(Access = private)                    
        % Dimensions of object
        dimensions 
        
        % Object type (scalar, row-vec, col-vec, matrix, tensor)
        dimension_type = 0
        
        % Cached real coefficients.
        real_coefs 
        
        % Cached complex coefficients.
        im_coefs  
        
        % Listener object for changes to underlying matrix system.
        symbol_added_listener = event.listener.empty;
        
        % True if co-efficients are cached.
        has_cached_coefs = false
        
        % True if co-efficients need zeros adding.
        needs_padding = false;
        
        % Object name in cache
        cached_object_name = string.empty(1,0);
    end
    
    properties(Constant, Access = protected)
        % Error: Mismatched scenario
        err_mismatched_scenario = ...
            'Cannot combine objects from different scenarios.';
        
        err_cannot_calculate = ...
            "calculateCoefficients not implemented for object of type '%s'";
    end
    
    
    %% Constructor
    methods
        function obj = ComplexObject(scenario, array_dimensions)
        % COMPLEXOBJECT Construct an object that evaluates to a complex number.
        %
        % PARAMS:
        %   scenario - The associated matrix system scenario.
        %
            arguments
                scenario (1,1) Abstract.Scenario
                array_dimensions (1,:) = {1, 1};
            end
            
            % Get dimensions argument
            if nargin < 2
                array_dimensions = [1, 1];
            elseif isnumeric(array_dimensions)
                array_dimensions = double(array_dimensions);
                
                % If vector, default to row vector
                if numel(array_dimensions) == 1
                    array_dimensions = [array_dimensions, 1];
                end
                
                array_dimensions = reshape(array_dimensions, 1, []);
            else
                error("Dimensions should be numeric, if provided.");
            end
            
            % All objects associated with same scenario
            obj = obj@handle();
            obj.Scenario = scenario;
            obj.dimensions = array_dimensions;       
            obj.setDimensionType();
            obj.resetCoefficients();            
        end
    end
    
    methods(Static)
        function obj = InitForOverwrite(scenario, array_dimensions)
            obj = Symbolic.ComplexObject(scenario, array_dimensions);
        end
    end
    
    %% Private derived properties
    methods (Access=private)
        function setDimensionType(obj)
            if numel(obj.dimensions) ~= 2
                obj.dimension_type = 4; % TENSOR
            elseif prod(obj.dimensions) == 1
                obj.dimension_type = 0; % SCALAR
            elseif obj.dimensions(1) == 1
                obj.dimension_type = 1; % ROW-VECTOR
            elseif obj.dimensions(2) == 1
                obj.dimension_type = 2; % COLUMN-VECTOR
            else
                obj.dimension_type = 3; % MATRIX
            end
        end
    end
    
    methods
        function val = DimensionType(obj)
            val = obj.dimension_type;
        end
        
        function val = IsScalar(obj)
            val = obj.dimension_type == 0;
        end
        
        function val = IsVector(obj)
            val = (obj.dimension_type == 1) || (obj.dimension_type == 2);
        end
        
        function val = IsRowVector(obj)
            val = (obj.dimension_type == 1);
        end
        
        function val = IsColVector(obj)            
            val = (obj.dimension_type == 2);
        end
        
        function val = IsMatrix(obj)
            val = (obj.dimension_type == 3);
        end
        
        function val = IsTensor(obj)
            val = (obj.dimension_type == 4);
        end
    end
      
    %% Co-efficients, and accessors
    methods
        function val = get.RealCoefficients(obj)
            % Build co-efficients if not cached.
            if ~obj.has_cached_coefs
                % Could throw:~
                obj.acquireCoefficients();
            end
            
            % Pad co-efficients with zero if necessary
            if obj.needs_padding
                obj.padCoefficients();
            end
            
            val = obj.real_coefs;
        end
        
        function val = get.ImaginaryCoefficients(obj)
            % Build co-efficients if not cached.
            if ~obj.has_cached_coefs
                % Could throw:~
                obj.acquireCoefficients();                
            end
            
            % Pad co-efficients with zero if necessary
            if obj.needs_padding
                obj.padCoefficients();
            end
            
            val = obj.im_coefs;
        end
        
        function val = Apply(obj, re_vals, im_vals)
        % APPLY Combine vector inputs with object coefficients.
        % Calculates: re_vals * re_coefs  + im_vals * im_coefs
        % 
        % PARAMS
        %   re_vals - (Row) vector of values to combine with real coefficients.
        %   im_vals - (Row) vector of values to combine with imaginary coefficients.
        %
        % RETURNS
        %  Object of the same type as re_vals; e.g. numeric or sdpvar.
        %  Shape of output matches size(obj).
        %
            arguments
                obj (:,:) Symbolic.ComplexObject
                re_vals (1,:)
                im_vals (1,:)
            end
           
            % Check and sanitize real inputs
            re_vals = reshape(re_vals, 1, []);
            if numel(re_vals) ~= obj.Scenario.System.RealVarCount
                error("%d real values expected, but %d were provided.", ...
                    obj.Scenario.System.RealVarCount, re_vals);
            end
           
            % Check and sanitize imaginary inputs if any
            if nargin >= 2
                has_imaginary = true;
                im_vals = reshape(im_vals, 1, []);
                if numel(im_vals) ~= obj.Scenario.System.ImaginaryVarCount
                    error("%d imaginary values expected, but %d were provided.", ...
                        obj.Scenario.System.ImaginaryVarCount, im_vals);
                end
            else
                has_imaginary = false;
            end
                    
            rec = obj.RealCoefficients;
            
            switch obj.dimension_type
                case 0 % SCALAR
                    val = re_vals * rec;
                case 1 % ROW_VECTOR
                    val = re_vals * rec;
                case 2 % COL_VECTOR
                    val = transpose(re_vals * rec);
                otherwise
                    error("Cannot yet apply to object of this type.");
            end
            
            if has_imaginary 
                imc = obj.ImaginaryCoefficients;
                switch obj.dimension_type
                    case 0 % SCALAR
                        val = val + (im_vals * imc);
                    case 1 % ROW_VECTOR
                        val = val + (im_vals * imc);
                    case 2 % COL_VECTOR
                        val = val + transpose(im_vals * imc);
                    otherwise
                        error("Cannot yet apply to object of this type.");
                end
            end            
        end
    end

    %% Private co-efficient caching / padding etc.
    methods(Access=private)
        function acquireCoefficients(obj)
        % ACQUIRECOEFFICIENTS Try to find out coefficients.
            try
                [obj.real_coefs, obj.im_coefs] = obj.calculateCoefficients();
                obj.has_cached_coefs = true;
                obj.needs_padding = false;
            catch error
                % Set to 0 state
                obj.resetCoefficients();
                
                % Fail
                rethrow(error);
            end
            
            % Register symbol update listener 
            if isempty(obj.symbol_added_listener)
                obj.symbol_added_listener = ...
                    obj.Scenario.System.listener('NewSymbolsAdded', ...
                                                 @obj.onNewSymbolsAdded);
            else
                obj.symbol_added_listener.Enabled = true;
            end
        end
                
        function resetCoefficients(obj)
        % RESETCOEFFICIENTS Forget calculated co-efficients.
            switch obj.dimension_type
                case 0 % SCALAR
                    obj.real_coefs = complex(sparse(1,0));
                    obj.im_coefs = complex(sparse(1,0));
                case 1 % ROW-VECTOR
                    obj.real_coefs = complex(sparse(0,0));
                    obj.im_coefs = complex(sparse(0,0));
                case 2 % COLUMN-VECTOR
                    obj.real_coefs = complex(sparse(0,0));
                    obj.im_coefs = complex(sparse(0,0));
                case 3 % MATRIX
                    obj.real_coefs = cell(obj.dimensions(2), 1);
                    obj.im_coefs = cell(obj.dimensions(2), 1);
                case 4 % TENSOR
                    obj.real_coefs = cell(obj.dimensions(2:end));
                    obj.im_coefs = cell(obj.dimensions(2:end));
            end
            
            obj.has_cached_coefs = false;
            obj.needs_padding = false;
            
            if ~isempty(obj.symbol_added_listener)
                obj.symbol_added_listener.Enabled = false;
            end           
        end
        
        function padCoefficients(obj)
        % PADCOEFFICIENTS Add zeros to end of coefficients.
            if ~obj.needs_padding
                return
            end
            
            assert(obj.has_cached_coefs);            
            switch(obj.dimension_type)
                case 0 % SCALAR
                    obj.padScalarCoefficients()
                case 1 % ROW-VECTOR
                    obj.padVectorCoefficients();
                case 2 % COL-VECTOR
                    obj.padVectorCoefficients();                    
                otherwise % MATRIX or TENSOR
                    obj.padTensorCoefficients();
            end
                  
            % Flag padding as done
            obj.needs_padding = false;
            
            % Start listening again for further updates
            if ~isempty(obj.symbol_added_listener)
                obj.symbol_added_listener.Enabled = true;
            end
        end
        
        function padScalarCoefficients(obj)
            arguments
                obj (1,1) Symbolic.ComplexObject
            end           
            % Pad real coefficients
            target_real = obj.Scenario.System.RealVarCount;
            delta_re = target_real - length(obj.real_coefs);
            if delta_re > 0
                obj.real_coefs = [obj.real_coefs; ...
                                  complex(sparse(delta_re, 1))];
            elseif delta_re < 0
                error("Real co-efficients should not shrink!");
            end
            
            % Pad imaginary coefficients
            target_im = obj.Scenario.System.ImaginaryVarCount;
            delta_im = target_im - length(obj.im_coefs);
            if delta_im > 0
                obj.im_coefs = [obj.im_coefs; ...
                                complex(sparse(delta_im, 1))];
            elseif delta_im < 0
                error("Imaginary co-efficients should not shrink!");
            end
        end
                
        function padVectorCoefficients(obj)
            % Pad real coefficients
            target_real = obj.Scenario.System.RealVarCount;
            delta_re = target_real - size(obj.real_coefs, 1);
            if delta_re > 0
                obj.real_coefs = [obj.real_coefs; ...
                                  complex(sparse(delta_re, numel(obj)))];
            elseif delta_re < 0
                error("Real co-efficients should not shrink!");
            end
            
            % Pad imaginary coefficients
            target_im = obj.Scenario.System.ImaginaryVarCount;
            delta_im = target_im - size(obj.im_coefs, 1);
            if delta_im > 0
                obj.im_coefs = [obj.im_coefs; ...
                                complex(sparse(delta_im, numel(obj)))];
            elseif delta_im < 0
                error("Imaginary co-efficients should not shrink!");
            end
        end
        
        function padTensorCoefficients(obj)            
            % Pad real coefficients
            target_real = obj.Scenario.System.RealVarCount;
            delta_re = target_real - size(obj.real_coefs{1}, 1);
            if delta_re > 0
                for idx=1:numels(obj.real_coefs)
                    obj.real_coefs{idx} = [obj.real_coefs{idx}; ...
                                  complex(sparse(delta_re, ...
                                                 obj.dimensions(1)))];
                end
            elseif delta_re < 0
                error("Real co-efficients should not shrink!");
            end
            
            % Pad imaginary coefficients
            target_im = obj.Scenario.System.ImaginaryVarCount;
            delta_im = target_im - size(obj.im_coefs{1}, 1);
            if delta_im > 0                
                for idx=1:numels(obj.im_coefs)
                    obj.im_coefs{idx} = [obj.im_coefs{idx}; ...
                                    complex(delta_im, ...
                                    sparse(obj.dimensions(1)))];
                end
            elseif delta_im < 0
                error("Imaginary co-efficients should not shrink!");
            end
        end
        
        function onNewSymbolsAdded(obj, ~, ~)
            assert(~isempty(obj.symbol_added_listener));            
            obj.needs_padding = true;
            obj.symbol_added_listener.Enabled = false;
        end
    end
    
       

    %% FIXME: MOVE ELSEWHERE, Power function overloading
    methods
        function val = mpower(lhs,rhs)
            arguments
                lhs (1,1) Symbolic.ComplexObject
                rhs (1,1) double
            end
        
            if rhs <= 0 || rhs ~= floor(rhs)
                error("Invalid exponent.");
            end

            val = lhs;
            for i=1:rhs-1
                val = mtimes(val, lhs);
            end
        end
    end

    %% Protected methods
    methods(Access=protected)
        function checkSameScenario(obj, other)
        % CHECKSAMESCENARIO Raise an error if scenarios do not match.        
            if obj.Scenario ~= other.Scenario
                error(obj.err_mismatched_scenario);
            end
        end
    end
    
    %% Indexing
    methods
        function varargout = size(obj, varargin)
        % SIZE The dimension of the object.
            if nargout <= 1
                if nargin <= 1
                    varargout{1} = obj.dimensions;
                else
                    if nargin > 2
                        error("Too many inputs to function size().");
                    end
                    if varargin{1} > numel(obj.dimensions)
                        varargout{1} = 1; % to match builtin size()
                    else 
                        varargout{1} = obj.dimensions(varargin{1});
                    end
                end
            else
                if nargin ~= 1
                    error("Too many outputs provided for size of one dimension.");
                end
                if nargout ~= numel(obj.dimensions)
                    error("Object has %d dimensions but %d outputs were provided.", ...
                          numel(obj.dimensions), nargout);
                end
                cell_dim = num2cell(obj.dimensions);
                [varargout{1:nargout}] = cell_dim{:};
            end
        end
        
        function val = numel(obj)
        % NUMEL Number of elements.
            val = prod(obj.dimensions);
        end
        
        function val = length(obj)
        % LENGTH Size of the longest dimension.
            val = max(obj.dimensions);
        end
        
        function varargout = subsref(obj,s)
            switch s(1).type
                case '.'
                    % Built-in can handle dot indexing
                    [varargout{1:nargout}] = builtin('subsref', obj, s);
                case '()'
                    % Do not currently handle logical indexing
                    if any(cellfun( @(x) isa(x, 'logical'), s(1).subs), 'all')                       
                        error("Logical indexing not supported.");
                    end
                    
                    % Replace ':' with 1:end etc.
                    indices = obj.cleanIndices(s(1).subs);
                    
                    if length(s) == 1
                        % Implement obj(indices)                        
                        varargout{1} = obj.splice(indices);
                        
                    elseif length(s) == 2 && strcmp(s(2).type,'.')                        
                        % Implement obj(indices).PropertyName
                        [varargout{1}, matched] = ...
                            obj.spliceProperty(indices, s(2).subs);
                        
                        if ~matched
                            error("Property %s not found.", s(2).subs);
                        end
                        
                    elseif length(s) == 3 && strcmp(s(2).type,'.') && strcmp(s(3).type,'()')
                        % Implement obj(indices).PropertyName(indices)
                        [property, matched] = ...
                            obj.spliceProperty(indices, s(2).subs);                        
                        if ~matched
                            error("Property %s not found.", s(2).subs);
                        end
                        [varargout{1:nargout}] = property(s(3).subs{:});                        
                    else                        
                        error('Not a valid indexing expression')
                    end
                case '{}'
                    error("Brace indexing is not supported for variables of this type."); 
                otherwise
                    error('Not a valid indexing expression')
            end
        end
        
        function n = numArgumentsFromSubscript(obj, s, indexingContext)
            n = 1;
        end
        
        function idx = end(obj, k, n)
        % END Gets the final index along dimension k.
            dims = size(obj);
            if k < n
                idx = dims(k);
            else
                % Potentially handle case where only 1 index is provided to
                % instead return 'numel'.
                idx = prod(dims(k:end));
            end
        end
        
        function val = split(obj)
        % SPLIT Splice object into equivalent-dimensioned array of scalars
            val = cell(size(obj));
            for idx=1:numel(val)
                val{idx} = obj.splice({idx});
            end
        end
            
            
    end
    
    %% Splicing
    methods(Access=private)
        function indices = cleanIndices(obj, indices)
        % CLEANINDICES Replace ':' with 1:end as necessary
            if numel(indices)==1
                if ischar(indices{1}) && indices{1} == ':'
                    indices{1} = 1:numel(obj);
                end
            else
                dims = size(obj);
                for idx = 1:numel(indices)
                    if ischar(indices{idx}) && indices{idx} == ':'
                        indices{idx} = 1:dims(idx);
                    end
                end
            end
        end
        
        function output = splice(obj, indices)
            % Target shape
            target_size = cellfun(@numel, indices);
            
            % Quick return when indexing into scalar
            if (prod(target_size) == 1) && obj.IsScalar
                assert(all(cellfun(@(x) x==1, indices)), ...
                       "Bad indexing of scalar object");
                output = obj;
                return;
            end
                        
            % Construct empty object
            output = feval(class(obj) + ".InitForOverwrite", ...
                           obj.Scenario, target_size);
                       
            % Copy properties
            output.spliceOut(obj, indices);            
        end            
    end
    
    %% Concatenation
    methods    
        function output = horzcat(varargin)      
            output = cat(2, varargin{:});
        end
      
        function output = vertcat(varargin)
            output = cat(1, varargin{:});
        end
        
        function output = cat(join_dimension, varargin)
            % Trivial cases:
            if nargin == 1
                output = Symbolic.ComplexObject.empty(0,0);
                return;
            elseif nargin == 2
                output = varargin{1};
                return;
            end
            
            % Semi-trivial cases (prune empty arrays!)
            mask = ~cellfun(@isempty, varargin);
            if ~any(mask, 'all')
                output = Symbolic.ComplexObject.empty(0,0);
                return;
            end
            varargin = varargin(mask);
            if numel(varargin) == 1
                output = varargin(1);
                return;
            end
            
            % Ensure all inputs are ComplexObjects
            if ~all(cellfun(@(x) isa(x, 'Symbolic.ComplexObject'), varargin), 'all')
                error("Can only concatenate Symbolic.ComplexObjects");
            end
            
            % Disable concatenation from different scenarios
            matching_scenario = all(cellfun(@(x) (varargin{1}.Scenario == x.Scenario), varargin), 'all');
            if ~matching_scenario                
                error("Can only concatenate Symbolic.ComplexObjects from the same scenario.");
            end
            
            % Disable hetrogenous concatenation
            homogenous = all(cellfun(@(x) strcmp(class(x), class(varargin{1})), varargin), 'all');
            if ~homogenous
                % TODO: Cast to polynomial, and concatenate
                error("Can only concatenate Symbolic.ComplexObjects of the same type.");
            end
            class_name = class(varargin{1});
                        
            % Get output dimensions (fail if inconsistent)
            sizes = cellfun(@size, varargin, 'UniformOutput', false);
            matching_tensor = all(numel(sizes{1}) == cellfun(@(x) numel(x), sizes), 'all');
            if ~matching_tensor
                error("Cannot merge tensors of different dimensionality.");
            end
            
            % Check matching sizes on dimensions that are not joined 
            nonjoin_dimensions = [1:(join_dimension-1), ...
                                  (join_dimension+1):numel(sizes{1})];
            cat_sizes = cellfun(@(x) x(join_dimension), sizes);
            non_cat_sizes = cellfun(@(x) x(nonjoin_dimensions), ...
                                    sizes, 'UniformOutput', false);            
            consistent = all(cellfun(@(x) (isequal(non_cat_sizes{1}, x)), non_cat_sizes), 'all');
            if ~consistent
                if numel(nonjoin_dimensions) == 1
                    if join_dimension == 1
                        error("Cannot vertically concatenate objects with different column sizes.");
                    else
                        error("Cannot horizontally concatenate objects with different row sizes.");                        
                    end
                else
                    error("Cannot concatenate objects with mismatched dimensions.");
                end
            end
            
            % Construct target size
            target_size = zeros(1, numel(sizes{1}));
            target_size(join_dimension) = sum(cat_sizes);
            [target_size(nonjoin_dimensions)] = non_cat_sizes{1};
            
            % Construct object offsets
            offsets = ones(numel(sizes{1}), numel(varargin));
            offsets(join_dimension, 1:end) = cumsum(cat_sizes);

            % Polymorphic c'tor
            output = feval(class_name + ".InitForOverwrite", ...
                           varargin{1}.Scenario, target_size);
                        
            % Do merge
            output.mergeIn(join_dimension, offsets, varargin);
    
        end
    end
    
    
    
    %% Virtual methods
    methods(Access=protected)
        function [re, im] = calculateCoefficients(obj)
        % CALCULATECOEFFICIENTS Overload this with actual calculation.  
        %
        % The format of the expected output depends on the dimension_type.
        % For scalar, outputs should be complex, sparse, column vectors.
        %
        % For row-vector and col-vector, outputs should be a complex,
        % sparse matrices; with each column defining one Scalar value
        % (transposes will happen elsewhere in the class for col-vecs).
        %
        % For matrices and tensors, output should be a cell array of
        % complex sparse matrices, with dimension 2 less than that of this
        % object.
        %
            error(obj.err_cannot_calculate, class(obj));
        end
        
        function spliceOut(output, source, indices)
        % SPLICEOUT
        % 
        
            % FIXME
            output.has_cached_coefs = false;
                        
            % Copy names
            if ~isempty(source.cached_object_name)
                output.cached_object_name = ...
                    source.cached_object_name(indices{:});
            end
        end
        
        function [output, matched] = spliceProperty(obj, indices, propertyName)
        % SPLICEPROPERTY Get properties from sub-index.
        % Subclasses should only call this function if they fail to match.
        % Only public properties need overloading here!
        
            switch propertyName
                case 'RealCoefficients'
                    % FIXME
                    error("RealCoefficients sub indexing not supported.");
                case 'ImaginaryCoefficients'
                    % FIXME
                    error("ImaginaryCoefficients sub indexing not supported.");
                case 'ObjectName'
                    output = obj.ObjectName(indices{:});
                    matched = true;
                case 'Scenario'
                    output = obj.Scenario;
                    matched = true;
                otherwise
                    matched = false;
            end
        end
        
        function merge_type = mergeIn(obj, merge_dim, offsets, objects)
        % MERGEIN Joins together complex objects, as part of concatenation.
        % Overloads should call this base function.
        % 
            assert(numel(objects)>=2);
            
            switch (objects{1}.dimension_type)            
                case 0 % Scalar -> 
                    if (obj.dimension_type == 1) % Scalar -> Row vector
                        merge_type = 0;
                    elseif (obj.dimension_type == 2) % Scalar -> Col vector  
                        merge_type = 1;
                    end
                
                case 1 % Row Vector -> 
                    if (obj.dimension_type == 1)
                        merge_type = 2; % Row vector -> Bigger row vector
                    else
                        merge_type = 4; % Row vector -> Matrix
                    end
                    
                case 2 % Col Vector ->
                    if (obj.dimension_type == 2)
                        merge_type = 2; % Col vector -> Bigger col vector
                    else
                        merge_type = 5; % Col vector -> Matrix
                    end
                    
                case 3
                    if (obj.dimension_type == 3)
                        merge_type = 6; % Matrix to matrix
                    else
                        merge_type = 7; % Matrix to tensor
                    end
                case 4
                    if numel(obj.dimensions) == numel(objects{1}.dimensions)
                        merge_type = 8; % Tensor to tensor of same dimension.
                    else
                        merge_type = 9; % Tensor to tensor of larger dimension.
                    end            
                otherwise
                    error("Unsupported concatenation type!");
            end
            
            
            % Only merge coefficients if we have all of them:
            if (all(cellfun(@(x) (x.has_cached_coefs), objects)))
                
                % Do padding on sub-elements, if required
                for idx = 1:numel(objects)
                    objects{idx}.padCoefficients();
                end
                
                % Combine scalars into row/col-vec,
                % or extend row-vec into bigger row-vec (resp. col-vec)
                if (merge_type >= 0) && (merge_type <= 3)
                    obj.real_coefs = objects{1}.real_coefs;
                    obj.im_coefs = objects{1}.im_coefs;
                    for idx = 2:numel(objects)
                        obj.real_coefs = [obj.real_coefs, ...
                                          objects{idx}.real_coefs];
                        obj.im_coefs = [obj.im_coefs, ...
                                        objects{idx}.im_coefs];
                    end
                end
                
                % Combine rows into matrix:
                if merge_type == 4                    
                    % Create empty vectors to recieve values
                    num_re = obj.Scenario.System.RealVarCount;
                    num_im = obj.Scenario.System.ImaginaryVarCount;
                    for col_id = 1:(obj.dimensions(2))
                        obj.real_coefs{col_id} = complex(sparse(num_re, 0));
                        obj.im_coefs{col_id} = complex(sparse(num_im, 0));
                    end
                    
                    % Now 'transpose' and deal, because of col-majority...
                    for row_id = 1:(obj.dimensions(1))
                        for col_id = 1:obj.dimensions(2)
                            obj.real_coefs{col_id} = ...
                                [obj.real_coefs{col_id}, ...
                                    objects{row_id}.real_coefs(:, col_id)];
                            obj.im_coefs{col_id} = ...
                                [obj.im_coefs{col_id}, ...
                                    objects{row_id}.im_coefs(:, col_id)];
                        end
                    end                        
                end
                
                % Combine columns into matrix:
                if merge_type == 5
                    % Due to col-majority, straightforward:
                    for col_id = 1:(obj.dimensions(2)) % NUMBER OF COLS
                        obj.real_coefs{col_id} = objects{col_id}.real_coefs;
                        obj.im_coefs{col_id} = objects{col_id}.im_coefs;
                    end
                end
                                                
                % FIXME: MATRIX AND TENSOR -> ???
                if merge_type >= 6
                    error("Tensor merges are not yet supported!");
                end
                
                % Register listener for symbol-table updates
                obj.has_cached_coefs = true;
                obj.needs_padding = false;
                obj.symbol_added_listener = ...
                    obj.Scenario.System.listener('NewSymbolsAdded', ...
                                                 @obj.onNewSymbolsAdded);
            end
        end       
    end
    
    
    
    %% Public CVX methods
    methods
        function cvx_expr = cvx(obj, real_basis, im_basis)
        % CVX Convert object to a (complex) CVX expression.
        %
        % Wraps Apply with checks for CVX expression inputs.
        %
        % See also: SYMBOLIC.COMPLEXOBJECT.APPLY
        %            
            if nargin >= 2
                if ~isa(real_basis, 'cvx')
                    error("Expected CVX vector for real basis input.");
                end
            else
                error("Expected at least one CVX basis vector input.");
            end
            
            if nargin >=3
                if ~isa(im_basis, 'cvx')
                    error("Expected CVX vector for imaginary basis input.");
                end                
                cvx_expr = obj.Apply(real_basis, im_basis);
            else
                cvx_expr = obj.Apply(real_basis);
            end              
        end
    end
    
    %% Public yalmip methods
    methods
        function ym_expr = yalmip(obj, real_basis, im_basis)
        % YALMIP Convert object to a (complex) YALMIP expression.
        %
        % Wraps Apply with checks for yalmip sdpvar object inputs.
        %
        % See also: SYMBOLIC.COMPLEXOBJECT.APPLY
        %
            if nargin >= 2
                if ~isa(real_basis, 'sdpvar')
                    error("Expected YALMIP sdpvar for real basis input.");
                end
            else
                error("Expected at least one YALMIP sdpvar basis input.");
            end
            
            if nargin >=3
                if ~isa(im_basis, 'sdpvar')
                    error("Expected YALMIP sdpvar for imaginary basis input.");
                end
                ym_expr = obj.Apply(real_basis, im_basis);
            else
                ym_expr = obj.Apply(real_basis);
            end
        end
    end
    
    %% TODO: Custom display
    methods
        function val = get.ObjectName(obj)
            if isempty(obj.cached_object_name)
                obj.cached_object_name = obj.makeObjectName();
            end
            val = obj.cached_object_name;               
        end
    end
    
    methods(Access=protected)
        function str = makeObjectName(obj)
        %MAKEOBJECTNAME Makes human-readable name for objects.
        % Should be overloaded by subclasses.
            dim = num2cell(size(obj));
            str = repelem("ComplexObject", dim{:});       
        end
    end
        
end

