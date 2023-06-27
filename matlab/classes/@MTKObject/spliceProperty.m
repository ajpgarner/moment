function [output, matched] = spliceProperty(obj, indices, propertyName)
% SPLICEPROPERTY Get properties from sub-index.
% Subclasses should only call this function if they fail to match.
% Only public properties need overloading here!

    switch propertyName
        case 'RealCoefficients'
            % FIXME
            error("RealCoefficients sub-indexing not supported.");
        case 'ImaginaryCoefficients'
            % FIXME
            error("ImaginaryCoefficients sub-indexing not supported.");
        case 'ObjectName'
            output = obj.ObjectName(indices{:});
            matched = true;
        case 'Scenario'
            output = obj.Scenario;
            matched = true;
        otherwise
            matched = false;
    end
end