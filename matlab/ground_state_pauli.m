function ground_state_pauli(n)

if nargin == 0
    n = 2;
end

level = 2;
coef_xx = 0.85;
coef_yy = -0.92;
coef_zz = 0.5;

setting = PauliScenario(n);

fprintf("Singles:\n");
tic
singles = setting.WordList(1);
singles = singles(2:end);
toc
fprintf("Doubles:\n");
tic
doubles = setting.WordList(2);
doubles = doubles(2:end);
toc

fprintf("Magnetization/objective:\n");
tic
X = MTKMonomial.InitZero(setting, [n 1]);
Y = MTKMonomial.InitZero(setting, [n 1]);
Z = MTKMonomial.InitZero(setting, [n 1]);

for i=1:n
    X(i) = singles(1 + 3*(i-1));
    Y(i) = singles(2 + 3*(i-1));
    Z(i) = singles(3 + 3*(i-1));
end

H = Z(n);
for i=1:n-1
    H = H + Z(i) + coef_xx*X(i)*X(i+1) + coef_yy*Y(i)*Y(i+1) + coef_zz*Z(i)*Z(i+1);
end
if n >= 3
    H = H + coef_xx*X(1)*X(n) + coef_yy*Y(1)*Y(n) + coef_zz*Z(1)*Z(n);
end

magnetization = Z(n);
for i=1:n-1
    magnetization = magnetization + Z(i);
end



objective = magnetization;
toc

fprintf("MM:\n");
tic
raw_mm = setting.MomentMatrix(level);
toc

fprintf("LM:\n")
tic
raw_lm_H = setting.LocalizingMatrix(H, level);
toc

fprintf("Gamma matrix:\n");
tic
minusHalfH = -0.5*H;
hmm = raw_mm * minusHalfH ;
mmh = minusHalfH * raw_mm;
raw_gamma = raw_lm_H + hmm + mmh;
toc

% 
% for i=1:n_ab
%     for j=i:n_ab
%         temp_p = monomials(i)'*H*monomials(j) - 0.5*(H*monomials(i)'*monomials(j) + monomials(i)'*monomials(j)*H);
%         if temp_p == 0
%             gamma(i,j) = 0;
%         else
%             temp_p = temp_p.ApplyRules(moment_rules);
%             gamma(i,j) = temp_p.yalmip(a,b);
%             if i ~= j
%                 gamma(j,i) = (gamma(i,j))';
%             end
%         end
%     end
% end

fprintf("Making rules:\n");
tic
monomials = doubles;
linear = monomials*H - H*monomials;
moment_rules = MomentRulebook(setting,"");
moment_rules.Add(linear, false);
toc

fprintf("Applying rules:\n");
tic
mm = raw_mm.ApplyRules(moment_rules);
gamma = raw_gamma.ApplyRules(moment_rules);

objective = objective.ApplyRules(moment_rules);
toc

[a,b] = setting.yalmipVars(); % NB:
M = mm.yalmip(a,b); 
G = gamma.yalmip(a,b);
objective = objective.yalmip(a,b);

constraints = [a(1) == 1, M >= 0];
%constraints = [constraints, M(1,1:7) == real(M(1,1:7))];
constraints = [constraints, G >= 0];

% Objective function (maximize)

% Solve
%ops = sdpsettings(sdpsettings,'verbose',1,'removeequalities',0,'solver','sdpa_gmp','sdpa_gmp.precision',200);
ops = sdpsettings(sdpsettings,'verbose',1,'solver','mosek','mosek.MSK_DPAR_INTPNT_CO_TOL_DFEAS',1e-12,'mosek.MSK_DPAR_INTPNT_CO_TOL_PFEAS',1e-12,'mosek.MSK_DPAR_INTPNT_CO_TOL_REL_GAP',1e-12,'mosek.MSK_DPAR_INTPNT_CO_TOL_MU_RED',1e-12);
fprintf("Solve:\n");
tic
optimize(constraints, objective,ops); 
toc


%value(M)
%check(constraints)
%g = value(gamma)
%g-g'
%[v d] = eig(g)

objective = value(objective)
explicit(n,coef_xx,coef_yy,coef_zz)

end

function e = explicit(n,coeff_xx,coeff_yy,coeff_zz)

x = [0 1;1 0];
y = [0 -1i;1i 0];
z = [1 0;0 -1];
xx = kron(x,x);
yy = kron(y,y);
zz = kron(z,z);

single = @(i) (kron(eye(2^(i-1)),kron(z,eye(2^(n-i)))));
joint = @(i) (kron(eye(2^(i-1)),kron(coeff_xx*xx + coeff_yy*yy + coeff_zz*zz,eye(2^(n-i-1)))));

H = single(n);
for i=1:n-1
    H = H + joint(i) + single(i);
end
if n >= 3
    H = H + coeff_xx*kron(x,kron(eye(2^(n-2)),x)) + coeff_yy*kron(y,kron(eye(2^(n-2)),y)) + coeff_zz*kron(z,kron(eye(2^(n-2)),z));
end
mag = single(n);
for i=1:n-1
    mag = mag + single(i);
end
[v d] = eig(H);
ground = v(:,1);
mag = single(n);
for i=1:n-1
    mag = mag + single(i);
end
correct = ground'*mag*ground
end

