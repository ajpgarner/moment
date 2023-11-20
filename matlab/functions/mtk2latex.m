function val = mtk2latex(input, varargin)
%MTK2LATEX Try to format an MTK object as a LaTeX expression

    if nargin < 1
        error("mtk2latex requires an input.");
    end
    
    % Polymorphic dispatch
    if isa(input, 'MTKOpMatrix')
        val = matrix_to_tex(input, varargin{:});      
    else
        error("Cannot convert object of type %s to LaTeX code.", ...
              class(input));
    end
    
end


%% Matrix output
function val = matrix_to_tex(input, varargin)
    % Set up
    endl = "\n";
    dims = size(input);
    seqs = input.SequenceStrings;

    % Generate
    val = "\\begin{bmatrix}" + endl;
    for row = 1:dims(2)
        if dims(1) == 0
            continue
        end
        
        val = val + clean_moment(seqs(row, 1));
        for col = 2:dims(1)
            val = val + " & ";
            val = val + clean_moment(seqs(row, col));
        end
        if row ~= dims(2)
            val = val + " \\\\ " + endl;
        else 
             val = val + endl;
        end
    end    
    val = val + "\\end{bmatrix}" + endl;
end

%% Utility
function str = clean_moment(input)
    str = strrep(input, "<", "\\expt{");
    str = strrep(str, ">", "}");
    str = strrep(str, ";", " ");
end