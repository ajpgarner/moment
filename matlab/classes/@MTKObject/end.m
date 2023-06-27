function idx = end(obj, k, n)
% END Gets the final index along dimension k.
    dims = size(obj);
    if k < n
        idx = dims(k);
    else
        % Final index can scale up to number of remaining elements
        idx = prod(dims(k:end));
    end
end