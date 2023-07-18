function output = commuting_sequences(operators, level)
%COMMUTING_SEQUENCES Generate all commuting sequences of a certain length.
%

    arguments
        operators (1,1) uint64
        level (1,1) uint64
    end

    % Level 0, just identity
    if level <= 0
       output = {uint64.empty(1,0)};
       return;
    end
    
    % Level 1, operator ids
    if level == 1
        output = cell(1, operators);
        for x = 1:operators
            output{x} = [x];
        end
        return;
    end
    
    % Otherwise, generalized triangle
    num_elems = nchoosek(operators+level-1, level); %multiset combo
    output = cell(1, num_elems);
    [output{:}] = deal(uint64(zeros(1, level)));
    
    for x=1:num_elems
        if x == 1
            output{x} = repmat([1], [1, level]);
            continue;
        end
        
        output{x} = output{x-1};
        recursing = true;
        cursor = level;
        while recursing            
            output{x}(cursor) = output{x}(cursor)+1;
            if output{x}(cursor) > operators
                for excess = cursor:level
                    output{x}(excess) = output{x}(cursor-1) + 1;
                end
                cursor = cursor - 1;
            else
                recursing = false;
            end
        end        
    end
end

