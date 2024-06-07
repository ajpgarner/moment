%% Example: yalmip_cglmp.m 
% Solves CGLMP inequality, with YALMIP
% Expected answer: (sqrt(33)-3)/9 (0.30495...)



%% Define number of outputs

d = 3;

% Two parties
scenario = LocalityScenario(2);
Alice = scenario.Parties(1);
Bob = scenario.Parties(2);

% Each party with two measurements
A0 = Alice.AddMeasurement(d);
A1 = Alice.AddMeasurement(d);
B0 = Bob.AddMeasurement(d);
B1 = Bob.AddMeasurement(d);

%% Make moment matrix
matrix = scenario.MomentMatrix(2);

% Make CGLMP object via Collins-Gisin notation
CGLMP = scenario.CGTensor(cglmp_cg(d));

%% Define and solve SDP
yalmip('clear')

% Get SDP vars and matrix
a = scenario.yalmipVars();
M = matrix.Apply(a);

% Constraints (normalization, positivity)
constraints = [a(1) == 1];
constraints = [constraints, M >= 0];

% Objective function (maximize)
objective = -CGLMP.Apply(a);

% Solve
optimize(constraints, objective); 

%% Get solutions
a = value(a);

%% Print out value found
cglmp_max_val = CGLMP.Apply(a)

function M = cglmp_cg(d)

oa = d;
ob = d;
ia = 2;
ib = 2;

V = zeros(oa-1,ob-1,ia,ib);

for a = 0:d-2
    for b = 0:d-2
	for x = 0:1
	    for y = 0:1
		if ((a+b -(d-2))*(-1)^((1-x)*(1-y)) >= 0)
	    	    V(a+1,b+1,x+1,y+1) = (-1)^(x*y);
		end
	    end
	end
    end
end

M = [0 -ones(1,d-1) zeros(1,d-1);
     -ones(d-1,1) V(:,:,1,1) V(:,:,1,2);
     zeros(d-1,1) V(:,:,2,1) V(:,:,2,2);];

end

