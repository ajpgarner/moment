function output = WordList(obj, length, register)
% WORDLIST Get all operator sequences up to requested length.
%
% PARAMS
%  length - The maximum length monomial to generate
%  register - Set true to register generated monomials as symbols.
%
    arguments
        obj(1,1) MTKScenario
        length(1,1) uint64
        register(1,1) logical = false
    end

    words = obj.WordListCell(length, register);
    
    % FIXME: More efficient call, premade with hashes
    output = MTKMonomial(obj, words, 1.0); 
end