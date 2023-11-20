clear
clear mtk;

qubit_count = 5;
base_mm_level = 2;

setting = PauliScenario(qubit_count);

MM = setting.MomentMatrix(base_mm_level, 1, false);
ref_id = setting.System.RefId;


X1 = setting.sigmaX(1);
Z1 = setting.sigmaZ(1);
p = X1+Z1;

lmX1 = setting.LocalizingMatrix(base_mm_level, X1, 1, false);
lmZ1 = setting.LocalizingMatrix(base_mm_level, Z1, 1, false);

lmP = setting.LocalizingMatrix(base_mm_level, p, 1, false);
%a = X1 * MM;