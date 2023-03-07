classdef Group < handle
    %GROUP Symmetry group
    
    properties(GetAccess=public,SetAccess=private)
        RepDimension % Dimension of matrix representation.
        Generators   % Matrix form of generators of the group.
        Elements     % Explicit matrix form of elements of the group.
    end
    
    properties(Dependent,GetAccess=public)
        Size         % Number of elements in the group.
        Average      % Average over all elements
    end
    
    methods
        function obj = Group(generators)
        %GROUP Construct a group, from its generators
            arguments
                generators (1,:) cell
            end
            
            % Take in generators
            obj.Generators = generators;
        
            % Empty generators -> group is just the identity
            if isempty(generators)
                obj.Elements = {1};
                obj.RepDimension = 1;
                return;
            end
            
            % Non empty generators, check all are same size
            obj_dims = size(generators{1});            
            obj.RepDimension = obj_dims(1);
            for idx = 1:length(generators)
                if ~ismatrix(generators{idx})
                   error("Generator at index %d was not a matrix.", idx); 
                end
                gen_size = size(generators{idx});
                if (gen_size(1) ~= gen_size(2))
                    error("Generator at index %d was not a square matrix.", idx);
                end                    
                if (gen_size(1) ~= obj.RepDimension)
                    error("Generator at index %d was of dimension %d (expected: %d)", ...
                        idx, gen_size(1), obj.RepDimension)
                end
            end
            obj.Elements = cell(1,0);
        end
        
        function Generate(obj, cycle_bound, tolerance)
        % GENERATE Enumerates group elements from generators.
        %
        % Uses Dimino's algorithm.
        %
        % PARAMS
        %   cycle_bound - Maximum size of a cycle before giving up.
        %   tolerance - Maximum distance between two numbers before
        %               assuming equality.
        %
            arguments
                obj (1,1) Symmetry.Group
                cycle_bound (1,1) uint64 = 10000
                tolerance (1,1) double = 100*eps(1)
            end
            
            obj.Elements = {eye(obj.RepDimension)};
            elem = obj.Generators{1};
            
            % First generate orbit of first generator
            while ~Util.is_close(elem, obj.Elements{1}, tolerance) ...
                    && length(obj.Elements) <= cycle_bound
                obj.Elements{end+1} = elem;
                elem = elem * obj.Generators{1};
            end
            if ~isequal(elem,obj.Elements{1})
                error("Cycle size of %d exceeded.", cycle_bound);
            end
            
            for gen_idx = 2:length(obj.Generators)
                next_gen = obj.Generators{gen_idx};            
                
                % Skip redundant generator
                if obj.Contains(next_gen, tolerance)
                   continue;
                end
                
                % Post multiply every element up to here:
                prev_order = length(obj.Elements);
                obj.Elements{end+1} = next_gen;       
                for prev_idx = 2:prev_order
                    prev_elem = obj.Elements{prev_idx};
                    obj.Elements{end+1} = prev_elem * next_gen;                
                end
                
                % First element of new coset
                rep_pos = prev_order + 1;
                while rep_pos < length(obj.Elements)
                    coset_rep = obj.Elements{rep_pos};                    
                    for other_gen_idx = 1:length(obj.Generators)
                        other_gen = obj.Generators{other_gen_idx};
                        elem = coset_rep * other_gen;
                        if ~obj.Contains(elem, tolerance)
                            obj.Elements{end+1} = elem;
                            for prev_idx = 2:prev_order
                                obj.Elements{end+1} = obj.Elements{prev_idx} * elem;
                            end
                        end
                    end                
                    rep_pos = rep_pos + prev_order;
                end               
            end
        end
        
        function val = get.Average(obj)
            if isempty(obj.Elements)
                error("Elements not generated.");
            end
            val = sum(cat(3,obj.Elements{:}),3)/length(obj.Elements);
        end
        
        function val = get.Size(obj)
            if isempty(obj.Elements)
                error("Elements not generated.");
            end
            val = uint64(length(obj.Elements));
        end
       
    end
    
    
    
    methods        
        function result = Contains(obj, elem, tolerance)
        % CONTAINS True, if supplied object is an element.
            result = any(cellfun(...
                         @(x) Util.is_close(x, elem, tolerance), ...
                         obj.Elements));
            
        end
    end
end

