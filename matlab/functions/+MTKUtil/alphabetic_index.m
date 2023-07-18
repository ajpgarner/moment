function [theString] = alphabetic_index(theIndex, isUpper, isZeroIndex)
% ALPHABETIC_INDEX Convert index/indices from numbers to letters.
% The letters are assigned in shortlex (excel-like) order: A-Z, AA-ZZ, etc.
    arguments
        theIndex (:,:) uint64
        isUpper (1,1) logical = true
        isZeroIndex (1,1) logical = false
    end

    flags = cell(1,0);
    if isUpper
        flags{end+1} = 'upper';
    else
        flags{end+1} = 'lower';
    end
    
    if nargin == 3
        if isZeroIndex
            flags{end+1} = 'zero_index';
        end
    end
    
    theString = mtk('alphabetic_name', theIndex, flags{:});        
end

