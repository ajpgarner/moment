 function varargout = getAll(obj)
% GETALL Creates one monomial object per fundamental operator.
%
% USAGE:
%    [x, y, z] = scenario.getAll();
%  
% RETURNS:
%    Number of monomials equal to OperatorCount, one per operator.
%
% See also: MTKMONOMIAL
%
     
    % Force generation of matrix system, if not already done
    obj.System;
    if obj.OperatorCount == 0
        error("No operators to get.");
    end

    export_conjugates = false;
    if nargout ~= obj.OperatorCount
        if ~obj.IsHermitian && nargout == 2*obj.OperatorCount
            export_conjugates = true;                    
        elseif nargout == 1
            % Create single output monomial 
            varargout = cell(1, 1);
            varargout{1} = MTKMonomial(obj, num2cell(1:obj.OperatorCount)');
            return;            
        else
            if obj.IsHermitian
                error("getAll() expects %d outputs.",...
                      obj.OperatorCount);
            else
                error("getAll() expects %d or %d outputs.",...
                      obj.OperatorCount, 2*obj.OperatorCount);
            end                        
        end

    end

    % Distribute output as varargout cell
    varargout = cell(1, nargout);
    if export_conjugates
        for index = 1:(2*obj.OperatorCount)
            varargout{index} = obj.get(index);
        end
    else
        if ~obj.IsHermitian && obj.Interleave
            for index = 1:obj.OperatorCount
                varargout{index} = obj.get((2*index)-1);
            end
        else
            for index = 1:obj.OperatorCount
                varargout{index} = obj.get(index);
            end
        end
    end
end