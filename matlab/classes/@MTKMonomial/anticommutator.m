function result = anticommutator(lhs, rhs)
%ANTICOMMUTATOR {A,B} = AB + BA

    % Alias:
    result = commutator(lhs, rhs, true);
end

