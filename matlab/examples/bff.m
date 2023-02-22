function key = bff()
%computes the the conditional von Neumann entropy using the Brown-Fawzi-Fawzi hierarchy arXiv:2106.13692

m=8;
chsh = 0.80;
value_ch = 2*chsh-1.5;

[w t] = gauss_radau(m);


key = (-1/m^2 + sum(w./t))/log(2);
for i=1:m-1
	key = key + (w(i)/(t(i)*log(2)))*hae(t(i),value_ch);
end

end

function objective = hae(t,value_ch)


mm_level = 2;

%% substitution rules
nops = 6;
hermitian = {{1+nops,1},{2+nops,2},{3+nops,3},{4+nops,4}};
proj = {{[1,1],[1]}, {[2,2],[2]}, {[3,3],[3]}, {[4,4],[4]}};
comm = {{[3,1],[1,3]},{[4,1],[1,4]},{[3,2],[2,3]},{[4,2],[2,4]}};

for j=1:4
	comm = [comm, {{[5,j],[j,5]},{[5+nops,j],[j,5+nops]}}];
	comm = [comm, {{[6,j],[j,6]},{[6+nops,j],[j,6+nops]}}];	
end

subs = [hermitian,proj,comm];

%% Create setting
setting = AlgebraicScenario(nops, subs, false);
%setting.Complete(20,true);


[A0, A1, B0, B1, Z0, Z1] = setting.getAll();

%% Make moment matrix
mm = setting.MakeMomentMatrix(mm_level);

%% Define and solve SDP
yalmip('clear');

    % Declare basis variables a (real)
    a = mm.yalmipVars;
    
    % Compose moment matrix from these basis variables
    M = mm.yalmipRealMatrix(a);
    

    % Objective
    ch = - A0 - B0 + A0*B0 + A0*B1 + A1*B0 - A1*B1;
    
    obj = A0*(Z0 + Z0' + (1-t)*Z0'*Z0) + t*Z0*Z0' + ...
    + Z1 + Z1' + (1-t)*Z1'*Z1 + t*Z1*Z1' ...    
    - A0*(Z1 + Z1' + (1-t)*Z1'*Z1);
        	
    constraints = [a(1) == 1;  M >= 0, ch.yalmip(a) >= value_ch];
    
     
	objective = obj.yalmip(a);
    ops = sdpsettings(sdpsettings,'verbose',0,'solver','mosek');
    optimize(constraints, objective, ops);
	objective = value(objective);
	
end


function [w t] = gauss_radau(m)

J = zeros(m,m);
for n=1:m-1
	J(n,n) = 0.5;
	J(n,n+1) = n/(2*sqrt(4*n^2-1));
	J(n+1,n) = J(n,n+1);
end
J(m,m) = (3*m-1)/(4*m-2);

[v d] = eig(J);

w = (v(1,:).^2)';
t=diag(d);

end	

