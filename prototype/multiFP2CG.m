function CG = multiFP2CG(p)

%takes a multipartite behaviour p in
%full probability notation V(a,b,c,...,x,y,z,...)
%to Collins-Gisin notation
% CG = [K   pB(0|1) p(0|2) ...
%	  pA(0|1) p(00|11) p(00|12) ...


desc = size(p)';
nparties = round(length(desc)/2);
outs = desc(1:nparties);
num_outs = prod(outs);

ins = desc(nparties+1:2*nparties);
num_ins = prod(ins);

p2cg = @(a,x) (a ~= outs).*( a + (x-1).*(outs-1)) + 1;

cgdesc = ins.*(outs-1)+1;
cgprodsizes = ones(length(cgdesc),1);
for i=1:length(cgdesc)
	cgprodsizes(i) = prod(cgdesc(1:i-1));
end
cgindex = @(posvec) cgprodsizes'*(posvec-1)+1;


prodsizes = ones(length(desc),1);
for i=1:length(desc)
	prodsizes(i) = prod(desc(1:i-1));
end
pindex = @(posvec) prodsizes'*(posvec-1)+1;

CG = zeros(cgdesc');

for inscalar = 0:num_ins-1
	invec = 1 + digits_mixed_basis(inscalar,ins,nparties);
	for outscalar = 0:num_outs-1
		outvec = 1 + digits_mixed_basis(outscalar,outs,nparties);
%		[outvec' invec' p2cg(outvec,invec)']
		for outscalar2 = 0:num_outs-1
			outvec2 = 1 + digits_mixed_basis(outscalar2,outs,nparties);
			if (outvec ~= outs).*outvec == (outvec ~= outs).*outvec2
				CG(cgindex(p2cg(outvec,invec))) = CG(cgindex(p2cg(outvec,invec))) + p(pindex([outvec2;invec]))/prod(ins(outvec == outs));
			end
		end
	end
end


end


function digits = digits_mixed_basis(ind,outs,nparties)

	digits = zeros(nparties,1);
	for i=nparties:-1:1
	    digits(i) = mod(ind,outs(i));
	    ind = floor(ind/outs(i));
	end

end
