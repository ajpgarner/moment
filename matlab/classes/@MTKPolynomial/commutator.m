function result = commutator(lhs, rhs, anticommute)
%COMMUTATOR [A,B] = AB - BA
% Elementwise commutation between LHS and RHS.
% Set third argument to true for anticommutator.
%   

    % Anti-commutation also possible
    if nargin < 3
        anticommute = false;
    end

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
        if ~anticommute
            result = MTKPolynomial.InitZero(size(this));
            return;
        end
    end
    
    % Attempt accelerated commutatation:
    if isa(other, 'MTKMonomial') || isa(other, 'MTKPolynomial')
        this.checkSameScenario(other);
        
        extra_args = cell(1,0);
        if anticommute
            extra_args{1} = 'anticommute';
        else
            extra_args{1} = 'commute';
        end

        % Operator cell multiplication
        [raw_result, is_mono] = mtk('commutator', ...
                                    this.Scenario.System.RefId, ...
                                    lhs.OperatorCell, rhs.OperatorCell, ...
                                    extra_args{:});

        if is_mono
            result = MTKMonomial.InitDirect(this.Scenario, raw_result{:});
        else
            result = MTKPolynomial.InitFromOperatorPolySpec(...
                        this.Scenario, raw_result);
        end
        return;
    end
    
    % Otherwise, calculate manually:
    if anticommute
        result = (lhs .* rhs) + (rhs .* lhs);
    else
        result = (lhs .* rhs) - (rhs .* lhs);    
    end
    
end
