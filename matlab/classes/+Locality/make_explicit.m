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
    parse_as_list = false;
    for idx = 1:numel(varargin)
        if strcmpi(varargin{idx}, "symbols")
            assert(~parse_as_list);
            parse_as_poly = false;
        elseif strcmpi(varargin{idx}, "polynomials")
            assert(~parse_as_list);
            assert(parse_as_poly);
        elseif strcmpi(varargin{idx}, "list")
            parse_as_poly = false;
            parse_as_list = true;
        elseif strcmpi(varargin{idx}, "simplify")
            extra_args{end+1} = "simplify";
        else
            error("Unrecognised flag '%s'", varargin{idx});
        end
    end
    
    assert(~(parse_as_poly && parse_as_list));
    if parse_as_poly
        extra_args{end+1} = "polynomials";
    elseif parse_as_list
        extra_args{end+1} = "list";
    else
        extra_args{end+1} = "symbols";
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
        val = MTKPolynomial.InitFromOperatorPolySpec(scenario, val);
        val.ReadOnly = true;
    end
end

