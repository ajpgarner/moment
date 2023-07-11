function [objective] = mtk_tsirelson(M, desc, level, toolkit)
% MTK_TSIRELSON Optimize a locality scenario to find the Tsirelson bound
% - M is the Bell functional, in Collins-Gisin form.
% - If all parties have the same number of measurements with the same number of outcomes
% desc is a vector of the form [parties, measurements, outcomes]
% if each party's measurements have the same number of outcomes
% desc is a vector of the form [a_outcomes, b_outcomes, ..., a_settings, b_settings, ...]
% otherwise desc is a cell where desc{i} is a vector with the number of outcomes of each
% of party i's measurements.
% - level is the depth of moment matrix to consider (default = 1)
% - toolkit is either 'yalmip' or 'cvx' (leave blank to automatically deduce).
% 
    % Validate M and desc
    if nargin < 2
        error("Collins-Gisin tensor and scenario description must be supplied.");
    end
       
    % Deduce moment matrix level 
    if nargin>=3
        level = uint64(level);
    else
        level = uint64(1);
    end

    % Deduce toolkit to use
    if nargin>=4
        toolkit = lower(toolkit);
        if ~ismember(toolkit, {'cvx', 'yalmip'})
            error("If specified, toolkit must be set to one of 'cvx' or 'yalmip'.");
        end
    else
        if Util.has_yalmip()
            toolkit = 'yalmip';
        elseif Util.has_cvx()
            toolkit = 'cvx';
        else
            error("Could not detect yalmip or cvx installation.");
        end
    end

    % Build scenario
    if isnumeric(desc) && length(desc) == 3
        scenario = LocalityScenario(desc(1), desc(2), desc(3));
    else
        scenario = LocalityScenario(desc);
    end
    
    % Make moment matrix and bell functional
    moment_matrix = scenario.MomentMatrix(level);
    bell_functional = scenario.CGTensor(M);
    
    % Invoke toolkit-specific SDP 
    if strcmp(toolkit,'yalmip')
        objective = tsirelson_yalmip(moment_matrix, bell_functional);
    elseif strcmp(toolkit, 'cvx')
        objective = tsirelson_cvx(moment_matrix, bell_functional);
    end

end

%% Private functions

function objective = tsirelson_yalmip(matrix, functional)
    % Define SDP variables, and represent moment matrix
    a = matrix.yalmipVars();
    Gamma = matrix.yalmipRealMatrix(a);

    % Constraints (normalization, positivity)
    constraints = [a(1) == 1, Gamma >= 0];

    % Objective function (maximize)
    objective = functional.yalmip(a);

    % Solve
    ops = sdpsettings(sdpsettings, 'verbose', 0, 'cachesolvers', 1);
    optimize(constraints, -objective, ops); 
    
    objective = value(objective);
end

function objective = tsirelson_cvx(matrix, functional)
    cvx_begin quiet sdp
        % Define SDP variables, and represent moment matrix
        matrix.cvxVars('a');
        Gamma = matrix.cvxRealMatrix(a);
   
        % Constraints (normalization, positivity)
        a(1) == 1;
        Gamma >= 0;
        
        % Objective function (maximize)
        the_objective = functional.cvx(a);        
        maximize(the_objective); 
    cvx_end
    
    objective = value(the_objective);   
end

