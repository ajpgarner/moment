function val = System(obj)
% SYSTEM Gets MatrixSystem object associated with scenario
%
% Will generate the MatrixSystem if it has not yet been
% created.
%
% RETURN:
%   A MatrixSystem object.
%
% See also: MatrixSystem
%
    arguments
        obj (1,1) MTKScenario
    end

    % Make matrix system, if not already generated
    if isempty(obj.matrix_system)
        obj.matrix_system = MatrixSystem(obj);
    end

    val = obj.matrix_system;
end