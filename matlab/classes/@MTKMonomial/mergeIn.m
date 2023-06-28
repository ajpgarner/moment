function mergeIn(obj, merge_dim, offsets, objects)
    merge_type = mergeIn@MTKObject(obj, merge_dim, offsets, objects);

    % If scalar, promote operator list to cell before merge.
    if merge_type >= 0 || merge_type <= 3
        m_operators = cell(1, numel(objects));
        
        for idx=1:numel(objects)
            if objects{idx}.IsScalar
                m_operators{idx} = {objects{idx}.Operators};
            else
                m_operators{idx} = objects{idx}.Operators;
            end
        end        
    else                                                   
        m_operators = (cellfun(@(x) x.Operators, objects, ...
                               'UniformOutput', false));
    end
    obj.Operators = cat(merge_dim, m_operators{:});

    m_coefficient = (cellfun(@(x) x.Coefficient, objects, ...
                             'UniformOutput', false));
    obj.Coefficient = cat(merge_dim, m_coefficient{:});

    m_hash = (cellfun(@(x) x.Hash, objects, 'UniformOutput', false));
    obj.Hash = cat(merge_dim, m_hash{:});

    m_symbol_id = (cellfun(@(x) x.symbol_id, objects, ...
                           'UniformOutput', false));
    obj.symbol_id = cat(merge_dim, m_symbol_id{:});

    m_symbol_conjugated = (cellfun(@(x) x.symbol_conjugated, objects, ...
                                   'UniformOutput', false));
    obj.symbol_conjugated = cat(merge_dim, m_symbol_conjugated{:});

    m_re_basis_index = (cellfun(@(x) x.re_basis_index, objects, ...
                                'UniformOutput', false));
    obj.re_basis_index = cat(merge_dim, m_re_basis_index{:});

    m_im_basis_index = (cellfun(@(x) x.im_basis_index, objects, ...
                                'UniformOutput', false));
    obj.im_basis_index = cat(merge_dim, m_im_basis_index{:});            
end        