function obj = InitFromOperatorCell(setting, cell_input)
    arguments
        setting (1,1) MTKScenario
        cell_input cell
    end
    dimensions = size(cell_input);
    obj = MTKPolynomial(setting, 'overwrite', dimensions);
    
    for idx=1:numel(cell_input)
        if numel(cell_input{idx}) == 3 
            obj.Constituents{idx} = ...
                MTKMonomial.InitDirect(setting, cell_input{idx}{:});            
        elseif numel(cell_input{idx}) == 7
            obj.Constituents{idx} = ...
                MTKMonomial.InitAllInfo(setting, cell_input{idx}{:});
        else 
            error("Error constructing element %d: expected 3 or 7 " + ...
                  "arrays to specify polynomial, but %d were provided", ...
                  idx, numel(cell_input{idx}));
        end
    end
    
    % De-cellify if scalar
    if obj.IsScalar
        assert(numel(obj.Constituents)==1);
        obj.Constituents = obj.Constituents{1};
    end
    
end