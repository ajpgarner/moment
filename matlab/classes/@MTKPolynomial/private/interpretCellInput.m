function output = interpretCellInput(setting, constituents)
%INTERPRETCELLINPUT Reads constituents cell array, and checks elements are
% either an MTKMonomial or a cell array that can be initialized into an
% MTKMonomial.
    
    % What elements are already monomials?
    entirely_mono = cellfun(@(x) isa(x, 'MTKMonomial'), constituents);
    
    % If entirely monomial, nothing more to do
    if all(entirely_mono(:))
       output = constituents;
       return;
    end
    
    % Otherwise, convert elements as necessary
    output = cell(size(constituents));    
    for idx = 1:numel(constituents)
        elem = constituents{idx};
        
        % Directly include monomials
        if entirely_mono(idx)
            output{idx} = elem;
            continue;
        end

        % Can we interpret as zero?
        if numel(elem) == 1
            if elem == 0
                output{idx} = MTKMonomial.empty(0, 1);
                continue;
            end
        end
        
        % Try to reinterpret as (partial/full) monomial specification
        if numel(elem) == 3
            output{idx} = MTKMonomial.InitDirect(setting, elem{:});
        elseif numel(elem) == 7 ...
                || numel(elem) == 8
            output{idx} = MTKMonomial.InitAllInfo(setting, elem{:});                            
        else
            error(MTKPolynomial.err_bad_ctr_input);
        end
    end
end

