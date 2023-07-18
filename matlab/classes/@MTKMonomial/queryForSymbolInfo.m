function [id, conjugated, re, im, aliased] = queryForSymbolInfo(obj)
% QUERYFORSYMBOLINFO Ask matrix system about symbols
    sys = obj.Scenario.System;
    
    maybe_aliased = obj.Scenario.PermitsSymbolAliases;

    if obj.IsScalar
        row = mtk('symbol_table', sys.RefId, obj.Operators);
        if isempty(row)
            [id, conjugated, re, im, aliased] = getDefaultSymbolInfo(obj);
        else
            id = int64(row.symbol);
            re = uint64(row.basis_re);            
            im = uint64(row.basis_im);
            conjugated = logical(row.conjugated);            
            if maybe_aliased 
                aliased = logical(row.is_alias);
            else
                aliased = false;
            end
         end
    else
        rows = mtk('symbol_table', sys.RefId, obj.Operators);
        
        % preallocate for speed
        [id, conjugated, re, im, aliased] = getDefaultSymbolInfo(obj);        
        for idx=1:numel(rows)
            row = rows(idx);
            if (row.symbol >= 0)
                id(idx) = row.symbol;
                re(idx) = uint64(row.basis_re);
                im(idx) = uint64(row.basis_im);
                conjugated(idx) = logical(row.conjugated);
                if maybe_aliased
                    aliased(idx) = logical(row.is_alias);
                end
            end
        end
        if ~maybe_aliased
            aliased = false(size(obj));
        end
    end
end

