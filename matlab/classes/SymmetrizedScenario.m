classdef SymmetrizedScenario < MTKScenario
%SYMMETRIZEDSCENARIO 
%

    properties(SetAccess = private, GetAccess = public)
        BaseScenario
        Group = Symmetry.Group.empty(0,0);
        MaxWordLength
    end
    
    %% Construction and initialization
    methods
        function obj = SymmetrizedScenario(base, generators, max_word_len)
        % SYMMETRIZEDSCENARIO Constructs a symmetrized version of a scenario.
        %
        % PARAMS:
        %   base         - The base scenario
        %   generators   - The generators of the symmetry group, as acting
        %                  on the fundamental operators of the scenario.
        %   max_word_len - The longest string that can be mapped into the
        %                  symmetrized scenario. Set to 0 to deduce.
        %
            arguments
                base (1,1) MTKScenario
                generators
                max_word_len (1,1) uint64 = 0
            end
            
            obj.BaseScenario = base;
            obj.Group = Symmetry.Group(obj, generators);
            obj.MaxWordLength = max_word_len;            
        end
    end
    
    %% Translation methods
    methods
        function output = Transform(obj, input)
            if isa(input, 'Abstract.RealObject')
                re_output = ...
                    mtk('transform_symbols', obj.System.RefId, ...
                         input.Coefficients);
                output = Abstract.RealObject(obj, reshape(re_output, 1,[]));
                return;
            end
            
            error("Object of type %s cannot be transformed", class(input));
        end
    end
    
    %% Virtual methods
    methods(Access={?MTKScenario,?MTKMatrixSystem})
        
        function ref_id = createNewMatrixSystem(obj)
        % CREATENEWMATRIXSYSTEM Invoke mtk to create imported matrix system.
            arguments
                obj (1,1) SymmetrizedScenario
            end
            
            base_id = obj.BaseScenario.System.RefId;

            ref_id = mtk('symmetrized_matrix_system', ...
                         base_id, obj.Group.Generators, ...
                         obj.MaxWordLength);
                     
             % TODO: Force base system to regenerate symbol tables
             
        end
    end
    
   
    
  
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) SymmetrizedScenario
                mm (1,1) OpMatrix.MomentMatrix
            end            
        end
    end
end

