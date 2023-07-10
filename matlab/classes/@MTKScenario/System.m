function val = System(obj)
% SYSTEM Gets MatrixSystem object associated with scenario
%
% Will generate the MatrixSystem if it has not yet been
% created.
%
% RETURN:
%   A MTKMatrixSystem object.
%
% See also: MTKMATRIXSYSTEM
%
    arguments
        obj (1,1) MTKScenario
    end

    % Make matrix system, if not already generated
    if isempty(obj.matrix_system)
        obj.matrix_system = MTKMatrixSystem(obj);
    end

    val = obj.matrix_system;
end