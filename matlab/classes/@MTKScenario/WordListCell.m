function output = WordListCell(obj, length, register)
% WORDLIST Get all operator sequences up to requested length.
%
% PARAMS
%  length - The maximum length monomial to generate
%  register - Set true to register generated monomials as symbols.
%

	if (nargin < 2) || (numel(length)~=1) || (length < 0)
		error("Must specify a positive maximum word length.");
	else
		length = uint64(length);
	end
	
	if nargin < 3
		register = false;
    else
        assert(numel(register) == 1 && islogical(register), ...
               "Register flag must be logical scalar (true or false).");
        register = logical(register);    
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