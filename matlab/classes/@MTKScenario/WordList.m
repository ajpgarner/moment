function output = WordList(obj, length, register)
% WORDLIST Get all operator sequences up to requested length.
%
% PARAMS
%  length - The maximum length monomial to generate
%  register - Set true to register generated monomials as symbols.
%
% RETURNS
%   Vector of MTKMonomials, one element for each word.
%

    if nargin < 2 || ~isnumeric(length) || length < 0
        error("Must specify a positive integer length.");
    else
        length = uint64(length);
    end
        
    if nargin < 3
        register = false;
    else
        assert(numel(register) == 1);        
        register = logical(register);
    end
   
    if register
        [ops, coefs, hashes, symbols, conj, real, im] = ...
            mtk('word_list', 'register_symbols', 'monomial', ...
                obj.System.RefId, length);
        
        obj.System.UpdateSymbolTable();            
            
        output = MTKMonomial.InitAllInfo(obj, ops, coefs, hashes, ...
                                         symbols, conj, real, im);
    else
        [ops, coefs, hashes] = mtk('word_list', 'monomial',...
                                   obj.System.RefId, length);
        output = MTKMonomial.InitDirect(obj, ops, coefs, hashes);
    end
end