function varargout = mtk_solve(matrices, objective, varargin)
% MTK_SOLVE Utility function for modelling and solving simple SDPs.
%
% SYNTAX:
%   1. [result, a, b] = mtk_solve(matrix);
%   2. [result, a, b] = mtk_solve(matrix, objective);
%
% PARAMETERS:
%  matrix - Should be a matrix object (e.g. MTKMomentMatrix), or a cell
%           array of matrix objects. One positive semi-definite constraint
%           will be added for each of these matrices.
%  objective - A polynomial that will be minimized. If not included (or set
%              to logical false), the SDP will instead be run in
%              feasibility check mode.
%
% OPTIONAL PARAMETERS:
%  complex -  Set to true to generate both real and imaginary components of
%             moments. Set to false to ignore imaginary components.
%  constraints - Set to an MTKPolynomial of additional constraints of the 
%                form P >= 0.
%  modeller - Set to 'cvx' or 'yalmip' to use one of these.
%  verbose  - Set to true to enable additional output.
%
% OUTPUTS:
% result - solved value of the objective if objective is set (as double), 
%          logical 'true' or 'false' in feasibility check mode.
% a (optional) - Final values of SDP variables associated with real
%                components of moments.
% b (optional) - Final values of SDP variables associated with imaginary
%                components of moments.
%

    % Check first parameter, and extract scenario
    if nargin < 1
        error("Must specify a matrix.");
    else
        [matrices, scenario] = parse_matrices(matrices);  
    end
    
    % Check objective, if set
    if nargin >= 2
        objective = parse_objective(scenario, objective);
    else
        objective = MTKPolynomial.empty(0,0);
    end

    % Get optional parameters
    opts = parse_optional_inputs(scenario, varargin{:});

    % Dispatch to appropriate modeller and solve
    if (strcmp(opts.modeller, 'cvx'))
        [result, a, b] = mtk_solve_cvx(scenario, objective, matrices, opts);
    elseif (strcmp(opts.modeller, 'yalmip'))
        [result, a, b] = mtk_solve_yalmip(scenario, objective, matrices, opts);
    else
        error("Unknown modeller '%s'.", opts.modeller);
    end
    
    % Report results
    if nargout == 0 || opts.verbose
        fprintf("Result: %f\n", result);        
    end  
    if nargout > 0 
        varargout = cell(1, nargout);
        if nargout >= 1
            varargout{1} = result;
        end
        if nargout >= 2
            varargout{2} = a;
        end
        if nargout >= 3
            varargout{3} = b;
        end
    end
end

%% Parse parameters
function [matrices, scenario] = parse_matrices(input_matrices)
  if isa(input_matrices, 'MTKOpMatrix')
        scenario = input_matrices.Scenario;
        matrices = {input_matrices};
        return;
  end
  
  if iscell(input_matrices)
        if isempty(input_matrices)
            error("First parameter should be a MTKMatrix or cell array of matrices.");
        end        
        ismtkm = cellfun(@(x) isa(x, 'MTKOpMatrix'), input_matrices);
        if ~all(ismtkm(:))
            error("First parameter should be a MTKMatrix or cell array of matrices.");
        end
        scenario = input_matrices{1}.Scenario;
        same_scenario = cellfun(@(x) scenario==x.Scenario, input_matrices);
        if ~all(same_scenario(:))
            error("All matrices should belong to the same scenario.");
        end
        matrices = input_matrices;
        return;
  end
  
  error("First parameter should be a MTKMatrix or cell array of matrices.");   
end

function objective = parse_objective(scenario, input_objective)
    if islogical(input_objective)
        if ~input_objective
            objective  = MTKPolynomial.empty(0,0);
            return;
        else
            error("Objective should be an MTKPolynomial, or false.");
        end
    end

    % Upcast if monomial
    if isa(input_objective, 'MTKMonomial')
        input_objective = MTKPolynomial(input_objective);
    end

    % Error, if not polynomial
    if ~isa(input_objective, 'MTKPolynomial')
        error("Objective must be a MTKPolynomial.");
    end
    if input_objective.Scenario ~= scenario
        error("Objective and all matrices must be from the same scenario.");
    end
    
    % Objective is okay...
    objective = input_objective;
end

function parsed = parse_optional_inputs(scenario, varargin)
    parsed = struct('complex', true, ...
                    'constraints', MTKPolynomial.empty(0,1), ...
                    'verbose', false, ...
                    'modeller', 'auto');

    % Parse options
    options = MTKUtil.check_varargin_keys( ...
        ["complex", "constraints", "modeller", "verbose"], ...
        varargin);

    for idx = 1:2:numel(varargin)
        switch options{idx}
            case 'constraints'
                parsed.constraints = ...
                    parse_constraints(scenario, options{idx+1});                
            case 'complex'
                if ~isscalar(options{idx+1}) || ~islogical(options{idx+1})
                    error("Complex should be true or false.");
                end
                parsed.complex = logical(options{idx+1});
            case 'modeller'
                toolkit = lower(char(options{idx+1}));
                if ~ismember(toolkit, {'cvx', 'yalmip'})
                    error("If specified, modeller must be set to one of 'cvx' or 'yalmip'.");
                end
                parsed.modeller = toolkit;
            case 'verbose'
                parsed.verbose = logical(options{idx+1});        
        end
    end
    
    % If no modeller, determine one automatically
    if strcmp(parsed.modeller, 'auto')
        if MTKUtil.has_yalmip()
            parsed.modeller = 'yalmip';
        elseif MTKUtil.has_cvx()
            parsed.modeller = 'cvx';
        else
            error("Could not detect yalmip or cvx installation.");
        end
    end

end

function val = parse_constraints(scenario, input_constraints)
    if ~isa(input_constraints, 'MTKPolynomial')
        error("Additional constraints should be an MTKPolynomial.");
    end
    if ~isempty(input_constraints)
        if input_constraints.Scenario ~= scenario
            error("Additional constraints should belong to the same scenario.");
        end
    end
    val = input_constraints;         
end


%% Solve using CVX
function [result, sdp_a, sdp_b] = mtk_solve_cvx(scenario, objective, ...
                                                matrices, opts)
    % Reset yalmip
    yalmip('clear');
    
    % Declare basis variables a (real)
    if opts.complex
        [a, b] = scenario.yalmipVars();
        
        % Normalization constraints
        constraints = [a(1) == 1];
        
        % Compose matrices with SDP constraints
        M = cell(numel(matrices), 1);
        for i = 1:numel(matrices)
            M{i} = matrices{i}.yalmip(a, b);
            constraints = [constraints, M{i} >= 0];            
        end

        % Extra polynomial constraints:
        C = cell(numel(opts.constraints), 1);
        for i = 1:numel(opts.constraints)
            C{i} = opts.constraints(i).yalmip(a, b);
            constraints = [constraints, C{i} >= 0];
        end
        
    else
        a = scenario.yalmipVars();
        
        % Normalization constraints
        constraints = [a(1) == 1];
        
        % Compose matrices with SDP constraints
        M = cell(numel(matrices), 1);
        for i = 1:numel(matrices)
            M{i} = matrices{i}.yalmip(a);
            constraints = [constraints, M{i} >= 0];            
        end
        
         % Extra polynomial constraints:
        C = cell(numel(opts.constraints), 1);
        for i = 1:numel(opts.constraints)
            C{i} = opts.constraints(i).yalmip(a);
            constraints = [constraints, C{i} >= 0];
        end
        
    end
        
    % Set other settings    
    yalmip_opts = sdpsettings('verbose', logical(opts.verbose));
    
    % Objective / feasibility
    if ~isempty(objective)
        if opts.complex        
            O = objective.yalmip(a, b);
        else 
            O = objective.yalmip(a);
        end        
        diagnostics = optimize(constraints, O, yalmip_opts);
        result = value(O);
    else
        diagnostics = optimize(constraints, yalmip_opts);
        if diagnostics.problem == 0
            result = true;
        elseif diagnostics.problem == 1
            result = false;
        else
            disp(diagnostics);
            error("Could not ascertain feasibility.");
        end
    end
	
    % Output SDP vars
    sdp_a = value(a);
    if opts.complex
        sdp_b = value(b);
    else
        sdp_b = double.empty(0,1);
    end        
end

%% Solve using yalmip
function [result, sdp_a, sdp_b] = mtk_solve_yalmip(scenario, objective, ...
                                                   matrices, opts)
    % Switch between verbose mode
    if opts.verbose
        modifiers = {'sdp'};
    else 
        modifiers = {'sdp', 'quiet'};
    end
    
    if opts.complex
        cvx_begin(modifiers{:})
            % Define SDP variables
            scenario.cvxVars('a', 'b');
            
            % Constraint: normalization
            a(1) == 1;

            % Constraint(s): SDP matrices
            M = cell(numel(matrices), 1);
            for i = 1:numel(matrices)
                M{i} = matrices{i}.cvx(a, b);
                M{i} >= 0;
            end
               
            % Extra polynomial constraints:
            C = cell(numel(opts.constraints), 1);
            for i = 1:numel(opts.constraints)
                C{i} = opts.constraints(i).cvx(a, b);
                C{i} >= 0;
            end

            % Set objective
            if ~isempty(objective)
                obj = objective.cvx(a, b);
                minimize obj;
            end
        cvx_end
    else
        cvx_begin(modifiers{:})
            % Define SDP variables
            scenario.cvxVars('a');
            
            % Constraint: normalization
            a(1) == 1;

            % Constraint(s): SDP matrices
            M = cell(numel(matrices), 1);
            for i = 1:numel(matrices)
                M{i} = matrices{i}.cvx(a);
                M{i} >= 0;
            end
            
            % Extra polynomial constraints:
            C = cell(numel(opts.constraints), 1);
            for i = 1:numel(opts.constraints)
                C{i} = opts.constraints(i).cvx(a);
                C{i} >= 0;
            end

            % Set objective
            if ~isempty(objective)
                obj = objective.cvx(a);
                minimize obj;
            end
        cvx_end
    end
       
    % Output is either objective, or feasibility status
    if ~isempty(objective)
        result = double(obj);
    else
        result = ~strcmp(cvx_status, 'Infeasible');
    end
    
    % Output SDP vars
    sdp_a = value(a);
    if opts.complex
        sdp_b = value(b);
    else
        sdp_b = double.empty(0,1);
    end
end