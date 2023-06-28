function output = WordListCell(obj, length, register)
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

    extra_args = cell(1,0);
    if register
        extra_args{end+1} = 'register_symbols';
    end

    output = mtk('word_list', obj.System.RefId, length, extra_args{:});

    if register
        obj.System.UpdateSymbolTable();
    end
end