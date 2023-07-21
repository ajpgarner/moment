classdef Group < handle
    %GROUP Symmetry group
    
    properties(GetAccess=public,SetAccess=private)
        Scenario     % Scenario that this group represents a symmetry on.
        Generators
    end
    
    properties(Dependent,GetAccess=public)
        Size         % Number of elements in the group.
    end
    
    properties(Access=private)
        representations % Explicit forms of group
    end
    
    methods
        function obj = Group(scenario, generators)
            % GROUP Construct a group, from its generators.
            %
            % PARAMS:
            %   scenario - Scenario object that the symmetry will act on.
            %   generators - Generators of the symmetry.
            %
   
			% Basic validation
			if nargin < 1 || ~isa(scenario, 'MTKScenario')
				error("First argument must be a scenario.");
			end
			if nargin < 2 || ~iscell(generators)
				error("Second argument should be provided as cell array");
			end
            
            % Record scenario
            obj.Scenario = scenario;
            
            % Empty generators -> group is just the identity
            if isempty(generators)
                error("Not yet supported");
            end
            
            % Non empty generators, check all are same size
            obj_dims = size(generators{1});
            gen_dim = uint64(obj_dims(1));
            for idx = 1:length(generators)
                if ~ismatrix(generators{idx})
                    error("Generator at index %d was not a matrix.", idx);
                end
                gen_size = size(generators{idx});
                if (gen_size(1) ~= gen_size(2))
                    error("Generator at index %d was not a square matrix.", idx);
                end
                if (gen_size(1) ~= gen_dim)
                    error("Generator at index %d was of dimension %d (expected: %d)", ...
                        idx, gen_size(1), gen_dim)
                end
            end
            
            % Save generators
            obj.Generators = generators;
            
            % Prepare representation storage
            obj.representations = containers.Map('KeyType', 'uint64',...
                                                  'ValueType', 'any');
        end
        
        function val = get.Size(obj)
            rep1 = obj.Representation(1);
            val = rep1.Size;
        end
        
        
        function val = Representation(obj, word_length)
        % REPRESENTATION Get representation for particular word length.
        %
        % PARAMS
        %   word_length - The polynomial order of the representation.
        %
            
			if nargin ~= 2
				error("Must specify word length for representation.");
			end
			word_length = uint64(word_length);
            
            % Already generated?
            if obj.representations.isKey(word_length)
                val = obj.representations(word_length);
                return
            end
            
            % Make representation           
            ref_id = obj.Scenario.System.RefId;
            new_rep_elems = mtk('make_representation', ...
                                ref_id, word_length);
            
            obj.representations(word_length) = ...
               Symmetry.Representation(obj, new_rep_elems);
           
           val = obj.representations(word_length);            
        end        
    end    
end

