function str = makeOperatorNames(obj)
%MAKEOPERATORNAMES Default operator names X1 ... XN
    
    if obj.IsHermitian
        str = "X" + 1:obj.OperatorCount;
    else
        str = "X" + 1:obj.OperatorCount;
        conj_str = "X" + 1:obj.OperatorCount + "*";
        if obj.Interleave            
            str = [str; conj_str];
            str = reshape(str, 1, []);
        else
            str = [str, conj_str];
        end
    end
        
end

