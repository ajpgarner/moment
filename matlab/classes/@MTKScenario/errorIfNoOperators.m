function errorIfNoOperators(obj)
%ERRORIFNOOPERATORS Raise an error if matrix system has no operators.
    if ~obj.DefinesOperators
        error(MTKScenario.err_no_ops, class(obj));
    end
end