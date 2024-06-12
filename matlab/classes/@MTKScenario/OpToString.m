function val = OpToString(obj, sequence)
% OPTOSTRING Format operator sequence as a string object
    
    % Validate
    if nargin < 2 || isempty(sequence)
        return
    end
	sequence = reshape(uint64(sequence), 1, []);
        
    str_array = obj.Rulebook.ToStringArray(sequence);
    val = join(str_array, " ");
end