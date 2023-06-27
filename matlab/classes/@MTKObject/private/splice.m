function output = splice(obj, indices)
%SPLICE Copy out sub-elements from this object.

    % Target shape
    target_size = cellfun(@numel, indices);

    % Quick return when indexing into scalar
    if (prod(target_size) == 1) && obj.IsScalar
        assert(all(cellfun(@(x) x==1, indices)), ...
               "Bad indexing of scalar object");
        output = obj;
        return;
    end

    % Construct empty object
    output = feval(class(obj) + ".InitForOverwrite", ...
                   obj.Scenario, target_size);

    % Copy properties
    output.spliceOut(obj, indices);            
end  