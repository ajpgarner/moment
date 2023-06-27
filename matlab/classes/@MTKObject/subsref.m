 function varargout = subsref(obj,s)
 %SUBSREF Subscripting
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