function val = OpToString(obj, sequence)
% OPTOSTRING Format operator sequence as a string object
    arguments
        obj MTKScenario
        sequence (1,:) uint64
    end
    
    if isempty(sequence)
        str= "I";
        return
    end
    
    strs = obj.OperatorStrings
    
    str_array = obj.Rulebook.ToStringArray(sequence);
    val = join(str_array, " ");
end