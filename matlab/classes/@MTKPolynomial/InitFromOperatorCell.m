function obj = InitFromOperatorCell(scenario, cell_input)
%INITFROMOPERATORCELL Constructs MTKPolynomial object from operator cells.
%
% SEE ALSO: MTKPolynomial.OperatorCell
%

	assert((nargin == 2) ...
		&& isa(scenario, 'MTKScenario') ...
		&& iscell(cell_input));
	
    dimensions = size(cell_input);
    
    % Special case 'empty':
    if isempty(cell_input)
        obj = MTKPolynomial.empty(dimensions);
        return;
    end
    
    % Make constituents
    constituents = cell(dimensions);
    for poly_index = 1:numel(cell_input)
        mono_count = numel(cell_input{poly_index});
        op_seqs = cell(mono_count, 1);
        weights = zeros(mono_count, 1);
        for mono_index = 1:mono_count
            op_seqs{mono_index} = cell_input{poly_index}{mono_index}{1};
            weights(mono_index) = cell_input{poly_index}{mono_index}{2};            
        end       
        constituents{poly_index} = MTKMonomial(scenario, op_seqs, weights);
    end
    
    
    % Construct:
    obj = MTKPolynomial(scenario, constituents);
    
end

