function obj = InitValue(scenario, values)
    if nargin ~=2 || ~isa(scenario, 'MTKScenario') || ~isnumeric(values)
        error("InitValue takes a scenario, and numeric values as input.");
    end

    % Synthesize values into symbol cell
    symbol_cell = cell(size(values));
    for idx=1:numel(values)
        symbol_cell{idx} = {{1, values(idx)}};
    end

    % Construct
    obj = MTKSymbolicObject(scenario, symbol_cell, 'raw');
end