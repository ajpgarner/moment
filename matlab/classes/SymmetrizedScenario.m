classdef SymmetrizedScenario < MTKScenario
%SYMMETRIZEDSCENARIO 
%

    properties(SetAccess = private, GetAccess = public)
        BaseScenario
        Group = Symmetry.Group.empty(0,0);
        MaxWordLength
        MaxSubgroupSize
        SymbolNameBase        
    end
    
    %% Error strings
    properties(Constant, Access=private)
        err_bad_transform = "Object of type %s cannot be transformed.";
    end
        
            
    %% Construction and initialization
    methods
        function obj = SymmetrizedScenario(base, generators, varargin)
        % SYMMETRIZEDSCENARIO Constructs a symmetrized version of a scenario.
        %
        % SYNTAX:
        % ss = SymmetrizedScenario(base, generators, [key1, val1,...])
        %
        %   base         - The base scenario
        %   generators   - The generators of the symmetry group, as acting
        %                  on the fundamental operators of the scenario.%
        % OPTIONAL PARAMS:
        %
        %   word_length  - The longest string that can be mapped into the
        %                  symmetrized scenario. Set to 0 to deduce.
        %   name         - The base variable name to use for the symmetrized 
        %                  symbols.
        %   max_subgroup - The largest subgroup allowed to appear in the
        %                  course of Dimino's algorithm applied to the
        %                  generators. (Used to prevent infinite loops from
        %                  incorrectly entered/ill-conditioned generators).
        %
        %
        
            % Check mandatory parameters
            if nargin < 2
                error("SymmetrizedScenario is defined by base scenario"...
                    + " and generators of the symmetry.");
            end
            if ~isa(base,'MTKScenario') || isa(base, 'SymmetrizedScenario')
                error("Base object must be (non-symmetrized) MTKScenario.");
            end
            
            % Parse optional parameters
            defines_tolerance = false;
            max_word_length = uint64(0);
            max_subgroup = uint64(0);
            name_base = "Y";            
            if ~isempty(varargin)
                options = MTKUtil.check_varargin_keys(...
                    ["name", "word_length", "tolerance"], varargin);                
                exclude_mask = false(size(options));
                for idx = 1:2:numel(varargin)
                    switch varargin{idx}
                        case 'name'
                            name_base = string(varargin{idx+1});
                            if ~isvarname(name_base)
                                error("Name base '%s' is not a valid "...
                                       + "variable name.", name_base); 
                            end
                            exclude_mask(idx:(idx+1)) = true;
                        case 'word_length'
                            max_word_length = uint64(varargin{idx+1});
                            if max_word_length < 0
                                error("Max word length must be nonnegative.");
                            end
                            exclude_mask(idx:(idx+1)) = true;
                        case 'max_subgroup'
                            max_subgroup = uint64(varargin{idx+1});
                            if max_word_length < 0
                                error("Max subgroup size must be nonnegative.");
                            end
                            exclude_mask(idx:(idx+1)) = true;
                        case 'zero_tolerance'
                            defines_tolerance = true;
                    end
                end
                
                if any(exclude_mask)
                    options = options(~exclude_mask);
                end
            else
                options = cell(1,0);
            end            
            options = [options, 'defines_operators', false];            
            if ~defines_tolerance
                options = [options, 'tolerance', base.ZeroTolerance];
            end
            
            % Call base constructor
            obj = obj@MTKScenario(options{:});
            
            % Save symmetrized options.
            obj.BaseScenario = base;
            obj.Group = Symmetry.Group(obj, generators);
            obj.MaxWordLength = max_word_length;
            obj.MaxSubgroupSize = max_subgroup;
            obj.SymbolNameBase = name_base;
        end
    end
    
    %% Translation methods
    methods
        function output = Transform(obj, input)            
            % Validate input is MTKObject from appropriate scenario
            if nargin < 2
                error("Must supply an object to transform.");
            end
            if ~isa(input, 'MTKObject')
                error(obj.err_bad_transform, class(input));
            end
            if input.Scenario ~= obj.BaseScenario
                error("Can only transform objects from the base scenario.");
            end
            
            % Alias for MomentMatrix
            if isa(input, 'MTKMomentMatrix')
                output = MTKMomentMatrix(obj, input.Level);                
                return;
            end
            
            % Alias for LM creation
            if isa(input, 'MTKLocalizingMatrix')
                output = MTKLocalizingMatrix(obj, input.Level, input.Word);
                return
            end
            
            % TODO: Other (e.g. substituted) matrix transform
            if isa(input, 'MTKOpMatrix')                
                error(obj.err_bad_transform, class(input));
            end
           
            % TODO: Monomials            
            if isa(input, 'MTKMonomial')
                error(obj.err_bad_transform, class(input));
            end
            
            % TODO: Polynomials
            if isa(input, 'MTKPolynomial')
                error(obj.err_bad_transform, class(input));
            end

%             
%                 re_output = ...
%                     mtk('transform_symbols', obj.System.RefId, ...
%                          input.Coefficients);
%                 output = Abstract.RealObject(obj, reshape(re_output, 1,[]));
%                 return;
%             end
            
            % Unknown object
            error(obj.err_bad_transform, class(input));
        end
    end
    
    %% Virtual methods
    methods(Access={?MTKScenario,?MTKMatrixSystem})
        
        function ref_id = createNewMatrixSystem(obj)
        % CREATENEWMATRIXSYSTEM Invoke mtk to create imported matrix system.
            
            % Get base system ID (possibly trigger base system generation).
            base_id = obj.BaseScenario.System.RefId;
            
            % Prepare extra params
            extra_args = cell(1,0);
            if obj.MaxWordLength > 0
                extra_args = [extra_args, ...
                              'max_word_length', obj.MaxWordLength];
            end
            if obj.MaxSubgroupSize > 0
                extra_args = [extra_args, ...
                              'max_subgroup', obj.MaxSubgroupSize];
            end
            if obj.ZeroTolerance ~= obj.BaseScenario.ZeroTolerance
                extra_args = [extra_args, 'tolerance', obj.ZeroTolerance];
            end
            
            % Make symmetrized system
            ref_id = mtk('symmetrized_matrix_system', ...
                         base_id, obj.Group.Generators, extra_args{:});
                     
            % Force base system to regenerate symbol tables
            obj.BaseScenario.System.UpdateSymbolTable();
        end
    end
    
    methods(Access=protected)
        function str = operatorCount(~)
            error("SymmetrizedScenario does not define operators.");
        end
        
        function str = makeOperatorNames(~)
            error("SymmetrizedScenario does not define operators.");
        end
        
        function val = onSetHermitian(~, old_val, ~)
            val = old_val;
        end
    end
end

