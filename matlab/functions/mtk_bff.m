function varargout = mtk_bff(scenario, bell_inequality, varargin)
%MTK_BFF Brown-Fawzi-Fazwi calculation on a locality scenario.

    % Validate inputs
    assert(nargout <= 3, "The outputs of mtk_bff are " ...
                       + "[result, scenario, Bell inequality]");
    assert(nargin >= 2, "Must supply a scenario and Bell inequality.");
    assert (isa(scenario,'LocalityScenario'), ...
            "The input scenario to mtk_bff must be a locality scenario.");
    assert(isa(bell_inequality, 'MTKPolynomial') ...
           || isa(bell_inequality, 'MTKMonomial'), ...
           "The input bell inequality must be a polynomial object.");
    assert(bell_inequality.Scenario == scenario, ...
           "The input bell inequality must belong to the input scenario.");
    
    % Get optional inputs, or defaults
    opts = parse_optional_inputs(scenario, varargin{:});
    
    % Some verbose output
    if opts.verbose
        tic
        fprintf("Preparing matrix system and adapting Bell inequality...");
    end
    
    % Adapt scenario
    if opts.global
        [bff_scenario, first_z] = adapt_scenario_global(scenario, opts);
    else
        [bff_scenario, first_z] = adapt_scenario_local(scenario, opts);
    end
    
    % Adapt Bell inequality into new scenario
    bff_bell = adapt_inequality(bell_inequality, bff_scenario);
    
    % Get a1, a(n-1) and 1 - sum(ai)
    alice_ops = get_alice_operators(scenario, bff_scenario, opts);
    
    % Some verbose output
    if opts.verbose
        timing = toc;
        fprintf(" done in %f seconds.\n", timing);
    end

    % Do calculation
    if opts.global
        result = solve_bff_global(bff_scenario, bff_bell, ... 
                               alice_ops, first_z, opts);
    else
        result = solve_bff_local(bff_scenario, bff_bell, ...
                              alice_ops, first_z, opts);
    end
    
    % Forward outputs
    if nargout>=1
        varargout = cell(1,nargout);
        varargout{1} = result;
        if nargout>=2
            varargout{2} = bff_scenario;
        end
        if nargout>=3
            varargout{3} = bff_bell;
        end
    end
end


%% Input handling
function parsed = parse_optional_inputs(scenario, varargin)
    parsed = struct('alice_input', 1, ...
                    'global', true, ...
                    'gr_steps', 8, ...                    
                    'mm_level', 2, ...
                    'verbose', false, ...
                    'modeller', 'auto');

    % Parse options
    options = MTKUtil.check_varargin_keys(...
        ["alice_input", "global", "gr_steps", ...
          "mm_level", "modeller", "verbose"],...
        varargin);

    for idx = 1:2:numel(varargin)
        switch options{idx}
            case 'alice_input'
                if ~isnumeric(options{idx+1}) ...
                    || ~isscalar(options{idx+1}) ...
                    || (options{idx+1} <= 0) ...
                    || (options{idx+1} ~= floor(options{idx+1})) ...
                    || (options{idx+1} > scenario.MeasurementsPerParty(1))
                    error("'alice_input' must be an integer in the range [1, %d].",...
                        scenario.MeasurementsPerParty(1));
                end
                parsed.alice_input = uint64(options{idx+1});
            case 'global'
                assert(islogical(options{idx+1}), ...
                    "'global' should be true or false");
                parsed.global = logical(options{idx+1});
            case 'gr_steps'
                if ~isnumeric(options{idx+1}) ...
                    || ~isscalar(options{idx+1}) ...
                    || (options{idx+1} <= 0) ...
                    || (options{idx+1} ~= floor(options{idx+1}))
                    error("'gr_steps' should be a positive integer.");
                end
                parsed.gr_steps = uint64(options{idx+1});
            case 'mm_level'
                if ~isnumeric(options{idx+1}) ...
                    || ~isscalar(options{idx+1}) ...
                    || (options{idx+1} <= 1) ...
                    || (options{idx+1} ~= floor(options{idx+1}))
                    error("'mm_level' should be an integer greater than or equal to 2.");
                end
                parsed.mm_level = uint64(options{idx+1});
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

%% Scenario adaption
function [bff, first_z] = adapt_scenario_local(scenario, opts)
    % First, get the number of outcomes for Alice's chosen measurement
    mmt = scenario.Parties(1).Measurements(opts.alice_input);
    outcomes = length(mmt.Outcomes);
    [bff, first_z] = adapt_scenario_with_extras(scenario, outcomes); 

end

function [bff, first_z] = adapt_scenario_global(scenario, opts)
    % First, get the number of outcomes for Alice's chosen measurement
    mmt = scenario.Parties(1).Measurements(opts.alice_input);
    outcomes = length(mmt.Outcomes);
    [bff, first_z] = adapt_scenario_with_extras(scenario, outcomes * opts.gr_steps);
end

function [bff, first_z] = adapt_scenario_with_extras(scenario, extra_ops)
    % Composer operator names
    op_names = scenario.OperatorNames;
    extra_names = "z" + (0:(extra_ops-1));

    % Make new scenario
    bff = AlgebraicScenario([op_names, extra_names], ...
                            'hermitian', false, ...
                            'interleave', true, ...
                            'normal', false, ...
                            'tolerance', scenario.ZeroTolerance);
                        
    % Make rules
    rules = bff.OperatorRulebook;
    
    bob_offset = (scenario.getOpIndex(2,1)-1)*2+1;
    z_offset = 2*length(op_names);
    first_z = z_offset+1;
    
    for l_op = 1:2:z_offset % Loop over locality operators
        rules.MakeHermitian(l_op)
        rules.MakeProjector(l_op)
    
        for z_op = 1:(2*numel(extra_names)) % Loop over added operators
           rules.AddCommutator(z_offset+ z_op, l_op)
        end
    end
    
    % Alice and Bob commute    
    for a_op = 1:2:(bob_offset-1)
        for b_op = (bob_offset):2:z_offset
            rules.AddCommutator(b_op, a_op);
        end
    end
    
    % Set should be automatically complete, but check
    assert(rules.Complete(1, false), ...
           "Something went wrong while formulating the scenario rulebook.");
end

%% Adapt Alice operators
function output = get_alice_operators(scenario, bff_scenario, opts)
    alice = scenario.Parties(1);
    mmt = alice.Measurements(opts.alice_input);
   
    first_op = scenario.getOpIndex(1, opts.alice_input);
    num_ops = numel(mmt.Outcomes)-1;
    
    % Make transformed outcomes for alice    
    tx_op_cell = cell(num_ops+1, 1);
    tx_op_cell{num_ops+1} = cell(1, num_ops+1);
    tx_op_cell{num_ops+1}{1} = {uint64.empty(1,0), 1.0};
    
    for i=1:num_ops
        input_op = first_op + i - 1;
        tx_op = uint64(((input_op -1)*2)+1);
        tx_op_cell{i} = {{tx_op, 1.0}};
        tx_op_cell{num_ops+1}{i+1} = {tx_op, -1.0};
    end
    output = MTKPolynomial.InitFromOperatorCell(bff_scenario, tx_op_cell);
  end

%% Bell inequality adaption
function bff_bell = adapt_inequality(bell_inequality, bff_scenario)

    % Upcast to polynomial if necessary
    if isa(bell_inequality, 'MTKMonomial')
        bell_inequality = MTKPolynomial(bell_inequality);
    end
    
    op_cell = cell(numel(bell_inequality.Constituents), 1);
    weights = zeros(numel(bell_inequality.Constituents), 1);
    
    for idx = 1:numel(bell_inequality.Constituents)
        ops = bell_inequality.Constituents(idx).Operators;
        op_cell{idx} = ((ops - 1) * 2) + 1;
        weights(idx) = bell_inequality.Constituents(idx).Coefficient;
    end
    
    monos = MTKMonomial(bff_scenario, op_cell, weights);
    bff_bell = MTKPolynomial(bff_scenario, monos);
    assert(bell_inequality.ObjectName == bff_bell.ObjectName);
end

function [w, t] = gauss_radau(m)
    m = double(m);
    J = zeros(m,m);
    for n=1:m-1
        J(n,n) = 0.5;
        J(n,n+1) = n/(2*sqrt(4*n^2-1));
        J(n+1,n) = J(n,n+1);
    end
    J(m, m) = (3*m-1)/(4*m-2);

    [v, d] = eig(J);

    w = (v(1,:).^2)';
    t = diag(d);
end	

%% Global solve
function val = solve_bff_global(scenario, bell_ineq, alice_ops, first_z, opts)
    if opts.verbose
        tic
        fprintf("Generating moment matrix level %d...", opts.mm_level);
    end
    mm = scenario.MomentMatrix(opts.mm_level);
    
    if opts.verbose
        timing = toc;
        dims = size(mm);
        fprintf(" done in %f seconds [%d x %d matrix].\n", ...
                 timing, dims(1), dims(2));
    end

    % TODO: Make objective function using w, t, etc.
    [w, t] = gauss_radau(opts.gr_steps);
    objective = MTKMonomial.InitZero(scenario, [1 1]);
    
    error("Global solve not yet implemented.");
        
    if strcmp(opts.modeller,'cvx')
        val = solve_sdp_yalmip(scenario, objective, bell_ineq, mm, opts);
    else
        val = solve_sdp_cvx(scenario, objective, bell_ineq, mm, opts);
    end   
end


%% Local solve
function val = solve_bff_local(scenario, bell_ineq, alice_ops, first_z, opts)
    % Make Moment Matrix
    if opts.verbose
        tic
        fprintf("Generating moment matrix level %d...", opts.mm_level);
    end
    mm = scenario.MomentMatrix(opts.mm_level);
    
    if opts.verbose
        timing = toc;
        dims = size(mm);
        fprintf(" done in %f seconds [%d x %d matrix].\n", ...
                 timing, dims(1), dims(2));
    end
    
    % Get G-R coefficients
    [w, t] = gauss_radau(opts.gr_steps);
    
    % Final 't==1' relaxation
    val = (-1/double(opts.gr_steps)^2 + sum(w./t))/log(2);
    
    % Remaining steps
    for gr_index=1:(opts.gr_steps-1)
        % Make objective function per step
        objective = MTKMonomial.InitZero(scenario, [1 1]);
        for i=1:numel(alice_ops)
            z = scenario.get((first_z - 1) + (i-1)*2 + 1);
            objective = objective + ...
                alice_ops(i) * (z + z' + (1-t(gr_index)) * (z' * z)) ...
                    + t(gr_index) * (z * z');
        end                
            
        % Solve and integrate
        weight = w(gr_index)/(t(gr_index)*log(2));
        val = val + ...
              weight * solve_sdp(scenario, objective, bell_ineq, mm, opts);
    end
end

%% Model and solve
function val = solve_sdp(scenario, objective, bell_ineq, mm, opts)
    if strcmp(opts.modeller, 'cvx')
        val = solve_sdp_cvx(scenario, objective, bell_ineq, mm, opts);
    else
        val = solve_sdp_yalmip(scenario, objective, bell_ineq, mm, opts);
    end   
end

function val = solve_sdp_yalmip(scenario, objective, bell_ineq, mm, opts)
    % Reset yalmip
    yalmip('clear');
    
    % Declare basis variables a (real)
    a = scenario.yalmipVars();
        
    % Compose objective
	O = objective.yalmip(a);
    
    % Compose moment matrix in these basis variables
    M = mm.yalmip(a);
        	
    % Compose constraints
    constraints = [a(1) == 1; M >= 0; bell_ineq.yalmip(a) >= 0];

    % Set other settings    
    yalmip_opts = sdpsettings('verbose', logical(opts.verbose));
    
    % Solve
    optimize(constraints, O, yalmip_opts);
	val = value(O);
end

function val = solve_sdp_cvx(scenario, objective, bell_ineq, mm, opts)
    if opts.verbose
        modifiers = {'sdp'};
    else 
        modifiers = {'sdp', 'quiet'};
    end
    
    cvx_begin(modifiers{:})
        scenario.cvxVars('a');
        
        M = mm.cvx(a);
        BI = bell_ineq.cvx(a);
        
        a(1) == 1;
        M >= 0;
        BI >= 0;        
        
        obj = objective.cvx(a);
        minimize obj;
    cvx_end
    val = double(obj);
end