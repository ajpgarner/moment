function result = is_close(lhs, rhs, tolerance)
%IS_CLOSE True, if matrices are close up to a tolerance.
    result = all(abs(lhs(:) - rhs(:)) <= tolerance);
end

