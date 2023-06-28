function errorIfLocked(obj)
% ERRORIFLOCKED Raise an error if matrix system is already created.
    if ~isempty(obj.matrix_system)
        error(obj.err_locked);
    end
end