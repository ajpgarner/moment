function [theString] = alphabetic_index(theIndex, isUpper, isZeroIndex)
%ALPHABETIC_INDEX Sequentially assign letters of the alphabet.
%   The letter assigned uses excel-like notation: A-Z, AA-ZZ, AAA-ZZZ, etc.
    if nargin < 3
        isZeroIndex = false;
    else
        isZeroIndex = logical(isZeroIndex);
    end
    
    if nargin < 2
        isUpper = true;
    else
        isUpper = logical(isUpper);
    end
    
    if nargin < 1
        error("At least one argument must be provided.");
    end
    
    if isZeroIndex
        if isUpper
            theString = npatk('alphabetic_name', 'zero_index', ...
                              'upper', theIndex);
        else
            theString = npatk('alphabetic_name', 'zero_index', ...
                              'lower', theIndex);
        end
    else
        if isUpper
            theString = npatk('alphabetic_name', 'upper', theIndex);
        else
            theString = npatk('alphabetic_name', 'lower', theIndex);
        end
    end
end

