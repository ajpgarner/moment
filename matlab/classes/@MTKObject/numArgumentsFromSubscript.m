function n = numArgumentsFromSubscript(obj, s, indexingContext)
%NUMARGUMENTSFROMSUBSCRIPT How many outputs should we expect?
   switch s(1).type
       case '.'
           % Built-in can handle most dot indexing
           n = builtin('numArgumentsFromSubscript', obj, s, indexingContext);
       otherwise
           n = 1;
   end
end
        