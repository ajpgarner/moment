function [output, matched] = spliceProperty(obj, indices, propertyName)
% SPLICEPROPERTY Get properties from sub-index.
% Subclasses should only call this function if they fail to match.
% Only public properties need overloading here!

    switch propertyName
        case 'RealCoefficients'            
            output = splice_coef_array(obj, indices, ...
                                       obj.RealCoefficients);
            matched = true;
        case 'ImaginaryCoefficients'
            output = splice_coef_array(obj, indices, ...
                                       obj.ImaginaryCoefficients);
            matched = true;
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

function val = splice_coef_array(obj, indices, coefs)
    flat_indices = sub2ind(size(obj), indices{:});
    val = coefs(:, flat_indices);
end