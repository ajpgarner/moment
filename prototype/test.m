% Probably should move this to C-function

U = int32([[1,2,3,6];[2,4,5,7];[3,5,6,8];[6,7,8,9]]);
[sorted, key, nu_diagonal, nu_elems] = sort_index_matrix(U)

