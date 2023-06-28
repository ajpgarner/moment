 function str = makeObjectName(obj)
    str = strings(size(obj));
    if obj.IsScalar
        str(1) = makeOneName(obj, obj.Constituents);
    else            
        for idx = 1:numel(obj)
            str(idx) = makeOneName(obj, obj.Constituents{idx});
        end
    end
 end

%% Private functions
function str = makeOneName(obj, constituents)
    if isempty(constituents)
        str = '0';
        return
    end

    str = constituents(1).ObjectName;
    for idx=2:numel(constituents)
        str = str + " + " + constituents(idx).ObjectName;
    end
end     