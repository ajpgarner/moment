function val = getWord(obj)
%GETWORD Effective localizing word of this matrix, if there is one.

    error('mtk:no_word', ...
          "%s does not define a localizing word.", class(obj));
end

