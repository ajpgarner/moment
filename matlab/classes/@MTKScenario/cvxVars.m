function cvxVars(obj, re_name, im_name)
%CVXVARS (Forward) Define CVX variables.
%
% Due to CVX's departure from functional programming, this must be called 
% from a scope where a cvx_problem is defined (i.e. between
% cvx_begin/cvx_end tags).
%
% See also: MTKMATRIXSYSTEM.CVXVARS
    
    if nargin <= 2
        im_name = char.empty;
    end
            
    % Check if exporting real, or real & imaginary
    export_imaginary = ~isempty(im_name);
    if export_imaginary
        if ~isvarname(im_name)
            error("Invalid variable name.");
        end
    end

    % Make CVX problem from caller scope visible in this scope.
    try
        cvx_problem = evalin( 'caller', 'cvx_problem', '[]');
        if ~isa( cvx_problem, 'cvxprob')
            error( "No CVX model exists in this scope.");
        end
    catch exception
        error( "No CVX model exists in this scope.");
    end

    % Export and forward
    if ~export_imaginary
        obj.System.cvxVars(re_name)
        assignin('caller', re_name, eval(re_name));
    else
        obj.System.cvxVars(re_name, im_name)
        assignin('caller', re_name, eval(re_name));
        assignin('caller', im_name, eval(im_name));
    end
    
end

