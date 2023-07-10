function val = make_explicit(scenario, free, fixed, ...
                             conditional, distribution, varargin)
% MAKE_EXPLICIT Make rules that impose a probablity distribution.
%
% This is a helper class to reduce code-duplication. Do not call directly. 
%
% Instead call Locality.Measurement.Probability,
% Locality.JointProbability.Probability, or
% Locality.JointProbability.ConditionalProbability.
%
   
    % Process probabilities
    distribution = reshape(double(distribution), 1, []);
    
    % Check if mode flag provided
    extra_args = cell(1,0);
    parse_as_poly = true;
    if numel(varargin) == 1
        if strcmpi(varargin{1}, "symbols")
            extra_args{end} = "symbols";
            parse_as_poly = false;
        else            
            extra_args{end+1} = "polynomials";
        end
    else
        extra_args{end+1} = "polynomials";
    end
    
    % Check if conditional
    if conditional
        extra_args{end+1} = "conditional";
    end
    
    % Invoke MTK
    if isempty(fixed)
        val = mtk('make_explicit', scenario.System.RefId, extra_args{:},...
                      free, distribution);
    else
        val = mtk('make_explicit', scenario.System.RefId, extra_args{:},...
                      free, fixed, distribution);
    end
    
    % Process output for polynomial mode
    if parse_as_poly    
        val = MTKPolynomial.InitFromOperatorCell(scenario, val);        
    end
end

