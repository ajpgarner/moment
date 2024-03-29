 function [output, matched] = spliceProperty(obj, indices, propertyName)
    switch propertyName
        case 'Operators'
            if obj.IsScalar
                output = obj.Operators;
            else
                output = obj.Operators(indices{:});
                if 1==numel(output)
                    output = output{1};
                end
            end
            matched = true;
        case 'Coefficient'
            output = obj.Coefficient(indices{:});
            matched = true;
        case 'Hash'
            output = obj.Hash(indices{:});
            matched = true;
        case 'FoundSymbol'
            output = obj.FoundSymbol(indices{:});
            matched = true;
        case 'SymbolId'
            output = obj.SymbolId(indices{:});
            matched = true;
        case 'SymbolConjugated'
            output = obj.SymbolConjugated(indices{:});
            matched = true;
        case 'RealBasisIndex'
            output = obj.RealBasisIndex(indices{:});
            matched = true;
        case 'ImaginaryBasisIndex'        
            output = obj.ImaginaryBasisIndex(indices{:});
            matched = true;
        case 'IsAlias'
            output = obj.IsAlias(indices{:});
            matched = true;
        otherwise
            [output, matched] = ...
                spliceProperty@MTKObject(obj, indices, propertyName);
    end
end