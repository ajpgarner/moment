%% EXAMPLE: brown_fawzi_fawzi.m
% Computes the conditional von Neumann entropy using the Brown-Fawzi-Fawzi 
% hierarchy [see arXiv:2106.13692].


%% Prepare setting
setting = make_bff_setting();
mm_level = 3;
gauss_radau_level = 8;
verbose = false;

chsh = 0.80;
value_chsh = 2*chsh-1.5;

%% Gauss Radau estimation
[w, t] = gauss_radau(gauss_radau_level);
val = (-1/gauss_radau_level^2 + sum(w./t))/log(2);
for i=1:gauss_radau_level-1
	val = val + (w(i)/(t(i)*log(2))) ...
             * solve_bff_sdp(setting, t(i), mm_level, value_chsh, verbose);
end

disp(val);

%% Make BFF setting
function setting = make_bff_setting()

    setting = AlgebraicScenario(["a0", "a1", "b0", "b1", "z0", "z1"], ...
                                'hermitian', false, 'normal', false);
    rules = setting.OperatorRulebook; 

    for op = rules.OperatorNames(1:4)
        rules.MakeHermitian(op);
        rules.MakeProjector(op);
        rules.AddCommutator("z0", op);
        rules.AddCommutator("z0*", op);
        rules.AddCommutator("z1", op);
        rules.AddCommutator("z1*", op);
    end

    rules.AddCommutator("b0", "a0");
    rules.AddCommutator("b1", "a0");
    rules.AddCommutator("b0", "a1");
    rules.AddCommutator("b1", "a1");

    % (NB: Redundant here, as rules are already complete.)
    setting.Complete(20);
end

%% Gets Gauss-Radau weights
function [w, t] = gauss_radau(m)
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

%% Define and solve BFF SDP
function val = solve_bff_sdp(setting, t, moment_matrix_level, ...
                             value_chsh, verbose)
    % Generate MM
    mm = setting.MomentMatrix(moment_matrix_level);
    
    [a0, a1, b0, b1, z0, z1] = setting.getAll();
    
    % CHSH constraint polynomial
    chsh = - a0 - b0 + a0*b0 + a0*b1 + a1*b0 - a1*b1;
    
    % Objective function polynomial   
    obj = a0*(z0 + z0' + (1-t)*(z0'*z0)) + t*(z0*z0') + ...
            + z1 + z1' + (1-t)*(z1'*z1) + t*(z1*z1') ...    
            - a0*(z1 + z1' + (1-t)*(z1'*z1));

    % Prepare yalmip
    yalmip('clear');

    % Declare basis variables a (real)
    a = setting.yalmipVars();
    
    % Compose moment matrix in these basis variables
    M = mm.yalmip(a);
        	
    % Impose constraints
    constraints = [a(1) == 1;  M >= 0, chsh.yalmip(a) >= value_chsh];
         
    % Set objective
	objective = obj.yalmip(a);
    
    % Set other settings    
    ops = sdpsettings(sdpsettings, 'verbose', verbose);
    
    % Solve
    optimize(constraints, objective, ops);
	val = value(objective);
end
