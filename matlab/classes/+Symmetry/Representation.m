classdef Representation
    %REPRESENTATION 
    
    properties(SetAccess=private,GetAccess=public)
        Dimension % Dimension of matrix representation.
        Elements % Matrix elements
        Group
    end
     
    properties(Dependent,GetAccess=public)
        Size         % Number of elements in the group.
        Average      % Average over all elements
    end
    
    methods
        function obj = Representation(group, elements)
 			
			if nargin < 1 || ~isa(group, 'Symmetry.Group')
				error("First argument must be a group.");
			end
			if nargin < 2 || ~iscell(elements)
				error("Second argument must be a cell array.");
            end
			
            obj.Group = group;
            
            if ~isempty(elements)
                dims = size(elements{1});
                obj.Dimension = dims(1);
                obj.Elements = elements;
            else
                obj.Dimension = 1;
                obj.Elements = {1};
            end
            
            % Validate elements
            for idx = 1:length(obj.Elements)
                if ~ismatrix(obj.Elements{idx})
                   error("Element at index %d was not a matrix.", idx); 
                end
                elem_size = size(obj.Elements{idx});
                if (elem_size(1) ~= elem_size(2))
                    error("Element at index %d was not a square matrix.", idx);
                end                    
                if (elem_size(1) ~= obj.Dimension)
                    error("Element at index %d was of dimension %d (expected: %d)", ...
                        idx, elem_size(1), obj.Dimension)
                end
            end
        end
    end
    
    methods
        function val = get.Average(obj)
            if isempty(obj.Elements)
                error("Elements not generated.");
            end
            val = sparse(obj.Dimension, obj.Dimension);
            for idx = 1:length(obj.Elements)
                val = val + obj.Elements{idx};
            end
            val = val / length(obj.Elements);            
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
                         @(x) MTKUtil.is_close(x, elem, tolerance), ...
                         obj.Elements));
            
        end
    end
end

