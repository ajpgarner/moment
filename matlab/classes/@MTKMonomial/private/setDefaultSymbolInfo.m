function setDefaultSymbolInfo(obj)
    if obj.IsScalar()
        obj.symbol_id = -1;
        obj.symbol_conjugated = false;
        obj.re_basis_index = 0;
        obj.im_basis_index = 0;
    else

        dims = size(obj);
        obj.symbol_id = int64(ones(dims)) * -1;
        obj.symbol_conjugated = false(dims);               
        obj.re_basis_index = uint64(zeros(dims));
        obj.im_basis_index = uint64(zeros(dims));
    end            
end