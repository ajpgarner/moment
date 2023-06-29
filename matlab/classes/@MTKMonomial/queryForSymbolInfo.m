function [id, conjugated, re, im] = queryForSymbolInfo(obj)
% QUERYFORSYMBOLINFO Ask matrix system about symbols
    sys = obj.Scenario.System;

    if obj.IsScalar
        row = mtk('symbol_table', sys.RefId, obj.Operators);
         if (isa(row, 'logical') && (row == false))
            [id, conjugated, re, im] = getDefaultSymbolInfo(obj);
        else
            id = int64(row.symbol);
            re = uint64(row.basis_re);
            im = uint64(row.basis_im);
            conjugated = logical(row.conjugated);
         end
    else
        rows = mtk('symbol_table', sys.RefId, obj.Operators);
        
        % preallocate for speed
        [id, conjugated, re, im] = getDefaultSymbolInfo(obj);        
        for idx=1:numel(rows)
            row = rows(idx);
            if (row.symbol >= 0)
                id(idx) = row.symbol;
                re(idx) = uint64(row.basis_re);
                im(idx) = uint64(row.basis_im);
                conjugated(idx) = logical(row.conjugated);                        
            end
        end
    end
end

