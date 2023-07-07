 function obj = subsasgn(obj, s, val)
    %SUBSASGN Subscript assignment
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
            if any(cellfun( @(x) isa(x, 'logical'), s(1).subs), 'all')                       
                error("Logical indexing not supported.");
            end
            
            % Only allow expressions of form: obj(index) = new_obj
            if length(s) ~= 1
                error("subsasgn can only be used to splice in whole elements.");
            end
  
            % Handle empty () case
            if isempty(s(1).subs)
                if isa(val, 'MTKObject') && isequal(size(obj), size(val))
                    obj = val;
                else
                    error("subsasgn can only be used to splice in whole elements.");
                end
                return;
            end
            
            % Replace ':' with 1:end etc.
            indices = obj.cleanIndices(s(1).subs);
            
            % Implement obj(index) = new_obj.
            if length(s) == 1
                obj.spliceIn(indices, val);
            else
                error("subsasgn can only be used to replace elements.");                
            end
        case '{}'
            error("Brace indexing is not supported for variables of this type."); 
        otherwise
            error('Not a valid indexing expression')
    end
 end