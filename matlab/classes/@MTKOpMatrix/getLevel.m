function val = getLevel(obj)
%GETLEVEL Gets generation/hierarchy level of this matrix if there is one.

    error('mtk:no_level', ...
          "%s does not define a hierarchy level.", class(obj));
end

