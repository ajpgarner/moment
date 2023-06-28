 function val = orderAndMerge(obj, monomials)
% ORDERANDMERGE Sort monomials, and combine repeated elements.

    assert(isa(monomials, 'MTKMonomial'));
    
    % Trivial case: empty
    if numel(monomials) == 0
        val = MTKMonomial.empty(0, 1);
        return;
    % Semi-trivial case: only one monomial (check it isn't zero!)
    elseif numel(monomials) == 1                
        [coef, m, mr, mi] = obj.Scenario.Prune(monomials.Coefficient);
        if m
            val = MTKMonomial.empty(0, 1);
        elseif mr || mi
            val = MTKMonomial(obj.Scenario, ...
                monomials.Operators, coef);
        else
            val = monomials;
        end
        return;
    end

    % Get Order
    [~, order] = sort(monomials.Hash);
    write_index = 0;

    % Merge construct
    op_cell = cell(0, 1);
    coefs = double.empty(0, 1);            
    last_hash = -1;

    for i = 1:numel(monomials)
        oid = double(order(i));
        next_hash = monomials.Hash(oid);
        if last_hash == next_hash
            coefs(write_index, 1) = coefs(write_index) ...
                        + monomials.Coefficient(oid);
        else
            write_index = write_index + 1;
            coefs(write_index, 1) = monomials.Coefficient(oid);
            op_cell{write_index, 1} = monomials.Operators{oid};
        end
        last_hash = next_hash;
    end

    % Identify and remove zeros
    [coefs, mask] = obj.Scenario.Prune(coefs);            
    if any(mask,'all')                
        if all(mask,'all')
            val = MTKMonomial.empty(0, 1);
            return;
        else
            coefs = coefs(mask);
            op_cell = op_cell(mask);
        end
    end

    % Make good polynomial
    val = MTKMonomial(obj.Scenario, op_cell, coefs);
end