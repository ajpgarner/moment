 function str = makeObjectName(obj)
    str = strings(size(obj));
    if obj.IsScalar
        str(1) = ...
            makeOneName(obj, obj.Operators, obj.Coefficient);
    else            
        for idx = 1:numel(obj)
            str(idx) = makeOneName(obj, obj.Operators{idx}, ...
                                        obj.Coefficient(idx));
        end
    end
 end

%% Private functions
function val = makeOneName(obj, opers, coef)
    if ~isempty(opers)
        
        if all(strlength(obj.Scenario.OperatorNames)==1)
            join_str = '';
        else
            join_str = ' ';
        end
            
        op_names = obj.Scenario.ToStringArray(opers);   
        val = join(op_names, join_str);

        if coef ~= 1.0
            if coef == -1.0
                val = "-" + val;
            else
                if ~isreal(coef)
                    if real(coef) == 0
                        if imag(coef) == 1
                            val = "i" + val;
                        elseif imag(coef) == -1
                            val = "-i" + val;
                        else
                            val = sprintf("%gi %s", imag(coef), val);
                        end
                    else
                        val = sprintf("(%g+%gi) %s", ...
                                      real(coef), imag(coef), val);
                    end
                else
                    val = sprintf("%g %s", coef, val);
                end
            end
        end
    else        
        if isreal(coef)
            val = sprintf("%g", coef);
        elseif real(coef) == 0
            if imag(coef) == 1
                val = "i";
            elseif imag(coef) == -1
                val = "-i";
            else
                val = sprintf("%gi", imag(coef));
            end
        else
            val = sprintf("g+%gi", real(coef), imag(coef));
        end        
    end
end