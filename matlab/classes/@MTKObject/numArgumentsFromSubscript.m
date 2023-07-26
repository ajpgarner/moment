function n = numArgumentsFromSubscript(obj, s, indexingContext)
%NUMARGUMENTSFROMSUBSCRIPT How many outputs should we expect?
   switch s(1).type
       case '.'
           % Built-in can handle /most/ but not all dot indexing            
            if obj.isPropertyMTKObject(s(1).subs)
                splice = builtin('subsref', obj, s(1));
                if isempty(splice)
                    n = 0;
                else
                    n = 1;
                end
            else
                n = builtin('numArgumentsFromSubscript', obj, s, indexingContext);
            end
       otherwise
           n = 1;
   end
end
        