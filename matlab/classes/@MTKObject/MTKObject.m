 classdef MTKObject < handle & matlab.mixin.CustomDisplay
% MTKOBJECT An object that can be evaluated to a complex number.
%
% A complex object is defined by its real and imaginary coefficients,
% corresponding to weightings to give to the real and imaginary basis
% elements in a scenario's symbol table.
%
% For solution column vectors 'a' and 'b', the value of a complex object is 
% given by "a . real_coefs + b . im_coefs".
%
    
    %% Public properties
    properties(GetAccess = public, SetAccess = private)
        % Associated scenario handle
        Scenario
    end
    
    %% Public, cached, properties
    properties(Dependent, GetAccess = public, SetAccess = private)
        % Human-readable description of object.
        ObjectName
        
        % Real coefficients as a complex row vector.
        RealCoefficients      
        
        % Imaginary coefficients as a complex row vector.
        ImaginaryCoefficients 
        
    end
        
    %% Private properties
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
    
    %% Error message strings
    properties(Constant, Access = protected)
        % Error: Mismatched scenario
        err_mismatched_scenario = ...
            'Cannot combine objects from different scenarios.';
        
        err_cannot_calculate = ...
            "calculateCoefficients not implemented for object of type '%s'";
    end
    
    
    %% Constructor
    methods
        function obj = MTKObject(scenario, array_dimensions)
        % COMPLEXOBJECT Construct an object that evaluates to a complex number.
        %
        % PARAMS:
        %   scenario - The associated matrix system scenario.
        %
            arguments
                scenario (1,1) MTKScenario
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
            obj.dimension_type = obj.calculateDimensionType();
            obj.resetCoefficients();            
        end
    end
    
    %% Named constructors
    methods(Static)
        function obj = InitForOverwrite(scenario, array_dimensions)
            obj = MTKObject(scenario, array_dimensions);
        end
    end

    %% Basic derived properties
    methods
        function val = DimensionType(obj)
            if isempty(obj)
                val = calculateDimensionType(obj);
            else
                val = obj.dimension_type;
            end
        end
        
        function val = IsScalar(obj)
            val = obj.DimensionType == 0;
        end
        
        function val = IsVector(obj)
            val = (obj.DimensionType == 1) || (obj.DimensionType == 2);
        end
        
        function val = IsRowVector(obj)
            val = (obj.DimensionType == 1);
        end
        
        function val = IsColVector(obj)            
            val = (obj.DimensionType == 2);
        end
        
        function val = IsMatrix(obj)
            val = (obj.DimensionType == 3);
        end
        
        function val = IsTensor(obj)
            val = (obj.DimensionType == 4);
        end
    end
      
    %% Dependent/cached property accessors
    methods
        function val = get.ObjectName(obj)
            if isempty(obj.cached_object_name)
                obj.cached_object_name = obj.makeObjectName();
            end
            val = obj.cached_object_name;               
        end
        
        function val = get.RealCoefficients(obj)
            % Build co-efficients if not cached.
            if ~obj.has_cached_coefs
                % Could throw:~
                acquireCoefficients(obj);
            end
            
            % Pad co-efficients with zero if necessary
            if obj.needs_padding
                padCoefficients(obj);
            end
            
            val = obj.real_coefs;
        end
        
        function val = get.ImaginaryCoefficients(obj)
            % Build co-efficients if not cached.
            if ~obj.has_cached_coefs
                % Could throw:~
                acquireCoefficients(obj);
            end
            
            % Pad co-efficients with zero if necessary
            if obj.needs_padding
                padCoefficients(obj);
            end
            
            val = obj.im_coefs;
        end
    end

    %% Event handlers
    methods(Access=private)      
        function onNewSymbolsAdded(obj, ~, ~)
            assert(~isempty(obj.symbol_added_listener));            
            obj.needs_padding = true;
            obj.symbol_added_listener.Enabled = false;
        end
    end
 
    %% Declaration of virtual methods (to be overloaded by child classes!)
    methods(Access=protected)
        [re, im] = calculateCoefficients(obj);

        spliceIn(obj, indices, value);
        
        spliceOut(output, source, indices);

        [output, matched] = spliceProperty(obj, indices, propertyName);

        merge_type = mergeIn(obj, merge_dim, offsets, objects);

        str = makeObjectName(obj)
        
        result = isPropertyMTKObject(obj, property_name);
    end
end