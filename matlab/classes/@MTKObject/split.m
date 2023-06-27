function val = split(obj)
% SPLIT Splice object into equivalent-dimensioned cell array of scalars.
    val = cell(size(obj));
    for idx=1:numel(val)
        val{idx} = obj.splice({idx});
    end
end