%% EXAMPLE: brown_fawzi_fawzi.m
% Computes the conditional von Neumann entropy using the Brown-Fawzi-Fawzi 
% hierarchy [see arXiv:2106.13692].
clear;
clear mtk;

%% Prepare setting
setting = make_bff_setting();
mm_level = 2;
gauss_radau_level = 8;
verbose = false;

chsh = 0.80;
value_ch = 2*chsh-1.5;

%% Gauss Radau estimation
[w, t] = gauss_radau(gauss_radau_level);
val = (-1/gauss_radau_level^2 + sum(w./t))/log(2);
for i=1:gauss_radau_level-1
	val = val + (w(i)/(t(i)*log(2))) ...
                * solve_bff_sdp(setting, t(i), mm_level, value_ch, verbose);
end

disp(val);

%% Make BFF setting
function setting = make_bff_setting()

    setting = AlgebraicScenario(["A0", "A1", "B0", "B1", "Z0", "Z1"], ...
                                {}, false, false);
    rules = setting.Rulebook; 

    for op = rules.OperatorNames(1:4)
        rules.MakeHermitian(op);
        rules.MakeProjector(op);
        rules.AddCommutator("Z0", op);
        rules.AddCommutator("Z0*", op);
        rules.AddCommutator("Z1", op);
        rules.AddCommutator("Z1*", op);
    end

    rules.AddCommutator("B0", "A0");
    rules.AddCommutator("B1", "A0");
    rules.AddCommutator("B0", "A1");
    rules.AddCommutator("B1", "A1");

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
                             value_ch, verbose)
    % Generate MM
    mm = setting.MakeMomentMatrix(moment_matrix_level);
    
    [A0, A1, B0, B1, Z0, Z1] = setting.getAll();
    
    % CHSH constraint polynomial
    ch = - A0 - B0 + A0*B0 + A0*B1 + A1*B0 - A1*B1;
    
    % Objective function polynomial
    obj = A0*(Z0 + Z0' + (1-t)*(Z0'*Z0)) + t*(Z0*Z0') + ...
            + Z1 + Z1' + (1-t)*(Z1'*Z1) + t*(Z1*Z1') ...    
            - A0*(Z1 + Z1' + (1-t)*(Z1'*Z1));
   
    % Prepare yalmip
    yalmip('clear');

    % Declare basis variables a (real)
    a = mm.yalmipVars;
    
    % Compose moment matrix in these basis variables
    M = mm.yalmipRealMatrix(a);
        	
    % Impose constraints
    constraints = [a(1) == 1;  M >= 0, ch.yalmip(a) >= value_ch];
         
    % Set objective
	objective = obj.yalmip(a);
    
    % Set other settings    
    ops = sdpsettings(sdpsettings,'verbose',verbose,'solver','mosek');
    
    % Solve
    optimize(constraints, objective, ops);
	val = value(objective);
end
