 function success = loadSymbolInfo(obj)
    % Set to default (and flag failure) if no matrix system yet.
    if ~obj.Scenario.HasMatrixSystem
         obj.setDefaultSymbolInfo();
         success = false;
         return;
    end

    sys = obj.Scenario.System;

    if obj.IsScalar
        row = mtk('symbol_table', sys.RefId, obj.Operators);
         if (isa(row, 'logical') && (row == false))
            obj.setDefaultSymbolInfo();
            success = false;
        else
            obj.symbol_id = int64(row.symbol);
            obj.re_basis_index = uint64(row.basis_re);
            obj.im_basis_index = uint64(row.basis_im);
            obj.symbol_conjugated = logical(row.conjugated);
            success = true;
         end
    else
        rows = mtk('symbol_table', sys.RefId, obj.Operators);
        obj.setDefaultSymbolInfo(); % preallocate for speed
        for idx=1:numel(rows)
            row = rows(idx);
            if (row.symbol >= 0)
                obj.symbol_id(idx) = row.symbol;
                obj.re_basis_index(idx) = uint64(row.basis_re);
                obj.im_basis_index(idx) = uint64(row.basis_im);
                obj.symbol_conjugated(idx) = logical(row.conjugated);                        
            end
        end
        success = all(obj.symbol_id >= 0);
    end
 end