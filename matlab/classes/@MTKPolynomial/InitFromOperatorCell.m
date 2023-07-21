function obj = InitFromOperatorCell(setting, cell_input)
    
	assert((nargin == 2) ...
		&& isa(setting, 'MTKScenario') ...
		&& iscell(cell_input));
	
    dimensions = size(cell_input);
    
    % Special case 'empty':
    if isempty(cell_input)
        obj = MTKPolynomial.empty(dimensions);
        return;
    end
    
    % Otherwise, construct:
    obj = MTKPolynomial(setting, 'overwrite', dimensions);
    
    if obj.IsScalar
        if numel(cell_input{1}) == 3
            obj.Constituents = ...
                MTKMonomial.InitDirect(setting, cell_input{1}{:});
        elseif numel(cell_input{1}) == 7
            obj.Constituents = ...
                MTKMonomial.InitAllInfo(setting, cell_input{1}{:});
        elseif setting.PermitsSymbolAliases && numel(cell_input{1}) == 8
            obj.Constituents = ...
                MTKMonomial.InitAllInfo(setting, cell_input{1}{:});
        else
            error("Error constructing polynomial: expected 3 or 7 " + ...
                "arrays to specify polynomial, but %d were provided", ...
                numel(cell_input{1}));
        end
        return;
    end
    
    for idx=1:numel(cell_input)
        if numel(cell_input{idx}) == 3 
            obj.Constituents{idx} = ...
                MTKMonomial.InitDirect(setting, cell_input{idx}{:});            
        elseif numel(cell_input{idx}) == 7
            obj.Constituents{idx} = ...
                MTKMonomial.InitAllInfo(setting, cell_input{idx}{:});
        elseif setting.PermitsSymbolAliases && numel(cell_input{idx}) == 8
            obj.Constituents{idx} = ...
                MTKMonomial.InitAllInfo(setting, cell_input{idx}{:});           
        else
            error("Error constructing element %d: expected 3 or 7 " + ...
                  "arrays to specify polynomial, but %d were provided", ...
                  idx, numel(cell_input{idx}));
        end
    end
  
end