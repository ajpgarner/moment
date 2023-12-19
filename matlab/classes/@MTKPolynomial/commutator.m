function result = commutator(lhs, rhs)
%COMMUTATOR [A,B] = AB - BA
% Elementwise commutation between LHS and RHS
%
    error("XXX: NOT YET IMPLEMENTED");
    
    % Pre-multiplication by a built-in type, or MTKMonomial
    if ~isa(lhs, 'MTKPolynomial')
        this = rhs;
        other = lhs;
    else
        this = lhs;
        other = rhs;
    end
    
    % Short-cut: commutation with real/complex numbers is always zero:
    if isnumeric(other)
        result = MTKPolynomial.InitZero(size(this));
        return;
    end
    
    % Attempt accelerated commutatation:
    if isa(other, 'MTKMonomial') || isa(other, 'MTKPolynomial')
        this.checkSameScenario(other); 

        % Operator cell multiplication
        raw_result = mtk('commutator', this.Scenario.RefId, ...
                         lhs.OperatorCell, rhs.OperatorCell);
        
    end
    
    % Otherwise, calculate manually:
    result = (lhs .* rhs) - (rhs .* lhs);    
end
