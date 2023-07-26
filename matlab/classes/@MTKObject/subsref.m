 function varargout = subsref(obj,s)
 %SUBSREF Subscripting
    switch s(1).type
        case '.'
            if length(s) == 1 || ~obj.isPropertyMTKObject(s(1).subs) || isempty(obj)
                % Built-in can handle /most/ dot indexing
                [varargout{1:nargout}] = builtin('subsref', obj, s);
            else
                % Otherwise, we must interpret recursively:
                last_mtko = obj;
                splice = obj;
                for idx = 1:(numel(s)-1)
                    if isempty(splice)
                        [varargout{1:nargout}] = builtin('subsref', splice, s(idx:end));
                        return;
                    elseif ~isequal(s(idx).type, '.') ...
                            || last_mtko.isPropertyMTKObject(s(idx).subs)
                        splice = subsref(splice, s(idx));
                        last_mtko = splice; 
                    else
                        splice = builtin('subsref', splice, s(idx));
                    end
                end

                if isempty(splice)
                    [varargout{1:nargout}] = builtin('subsref', splice, s(end));
                else
                    [varargout{1:nargout}] = subsref(splice, s(end));
                end
            end
        case '()'
            % Handle empty () case by ignoring indices.
            if isempty(s(1).subs)
                if length(s) == 1
                    varargout{1} = obj;
                else
                    [varargout{1:nargout}] = subsref(obj, s(2:end));
                end
                return;
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
                error('Not a valid indexing expression.')
            end
        case '{}'
            error("Brace indexing is not supported for variables of this type."); 
        otherwise
            error('Not a valid indexing expression.')
    end
 end
