n = 5;

level = 2;
coef_xx = 0.85;
coef_yy = -0.92;
coef_zz = 0.5;

setting = PauliScenario(n);

singles = setting.WordList(1);
singles = singles(2:end);
doubles = setting.WordList(2);
doubles = doubles(2:end);

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

mm = setting.MomentMatrix(level);
setting.Symbols
%momo = Z(2)*Y(3);
%bad = momo*H-H*momo
%bad.RealCoefficients
%bad.ImaginaryCoefficients

