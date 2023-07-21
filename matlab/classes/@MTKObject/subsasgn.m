 function obj = subsasgn(obj, s, val)
 %SUBSASGN Subscript assignment
 
    % Check if read only
    if obj.read_only
        error("Cannot assign to read-only object.");
    end
 
    % Do assignment
    switch s(1).type
        case '.'
            if length(s) == 1 || ~obj.isPropertyMTKObject(s(1).subs)
                % Built-in can handle /most/ dot indexing
                obj = builtin('subsasgn', obj, s, val);
            else
                error("subsasgn not supported.");
            end
        case '()'
            % Do not currently handle logical indexing
            logical_indices = cellfun( @(x) isa(x, 'logical'), s(1).subs);
            if any(logical_indices(:))
                error("Logical indexing not supported.");
            end
            
            % Only allow expressions of form: obj(index) = new_obj
            if (length(s) ~= 1) || ~isa(val, 'MTKObject')
                error("subsasgn can only be used to splice in MTKObjects.");
            end
  
            % Handle empty () case
            if isempty(s(1).subs)
                if isequal(size(obj), size(val))
                    obj = val;
                else
                    error("A() = B requires B to be the same size as A."...
                         + " Use A = B for dimension-changing overwrite.");
                end
                return;
            end
            
            % Replace ':' with 1:end etc.
            indices = obj.cleanIndices(s(1).subs);
            
            % Implement obj(index) = new_obj.
            obj.spliceIn(indices, val);
            
        case '{}'
            error("Brace indexing is not supported for variables of this type."); 
        otherwise
            error('Not a valid indexing expression')
    end
 end
