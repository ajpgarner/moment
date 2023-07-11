function val = combineToLM(lhs, rhs, negate_rhs)
%COMBINETOLM Combine two operator matrices additively to make new matrix.
% If matrices cannot be combined, returns false.

    assert(lhs.Scenario == rhs.Scenario);  
    
    % If matrices are not same size, immediately fail:
    if ~isequal(size(lhs), size(rhs))
        val = false;
        return;
    end

    % If operator matrices do not define words, fail:
    try
        lhs_word = lhs.getWord();
        lhs_level = lhs.getLevel();
        rhs_word = lhs.getWord();
        rhs_level = rhs.getLevel();
    catch Exception
        if ~strcmp(Exception.identifier, 'mtk:no_word') && ...
           ~strcmp(Exception.identifier, 'mtk:no_level')
            rethrow(Exception);
        end
        val = false;
        return;
    end

    % If matrix levels are somehow different, fail:
    if lhs_level ~= rhs_level
        val = false;
        return;
    end

    % Add words, and make new LM
    if negate_rhs
        combined_word = lhs_word - rhs_word;
    else
        combined_word = lhs_word + rhs_word;
    end
    val = MTKLocalizingMatrix(lhs.Scenario, lhs_level, combined_word);

end

