classdef SolvedMomentMatrix < handle
    %SOLVEDMOMENTMATRIX A matrix of operator products, evaluated after
    % an SDP solve.
    properties(SetAccess = protected, GetAccess = public)
        a
        b
        isComplex = true
        SymbolTable
        MomentMatrix
    end
    
    methods
        function obj = SolvedMomentMatrix(moment_matrix, ...
                                          real_sol, im_sol)
            arguments
                moment_matrix (:,:) MomentMatrix
                real_sol (:,1) {isNumericOrSDPvar}
                im_sol (:,1) {isNumericOrSDPvar} = double.empty(0,1) 
            end 
            obj.MomentMatrix = moment_matrix;
            
            % Check and copy real solution
            exp_re_length = moment_matrix.MatrixSystem.RealVarCount;
            if (exp_re_length ~= length(real_sol))
                error("Real solution vector length " ...
                     + "( " + num2str(exp_re_length) + ") " ...
                     + "doesn't match basis size " ...
                     + "( " + num2str(length(real_sol)) + ").");
            end
            
            % Get numeric value of real part
            if isnumeric(real_sol)
                real_sol_vals = real_sol;
            elseif isa(real_sol, 'sdpvar')
                real_sol_vals = obj.extract_yalmip_value(real_sol);
            end
            
            obj.a = real_sol_vals;
            
            % Check and copy imaginary solutions, if set
            if (nargin >= 3)    
                exp_im_length = moment_matrix.MatrixSystem.ImaginaryVarCount;
                if (exp_im_length ~= length(im_sol))
                    error("Imaginary solution vector length " ...
                     + "( " + num2str(exp_im_length) + ") " ...
                     + "doesn't match basis size " ...
                     + "( " + num2str(length(im_sol)) + ").");
                end            
                
                % Get numeric value of imaginary part
                if isnumeric(real_sol)
                    im_sol_vals = im_sol;
                elseif isa(real_sol, 'sdpvar')
                    im_sol_vals  = obj.extract_yalmip_value(im_sol);
                end

                obj.b = im_sol_vals;
                obj.isComplex = ~isempty(obj.b);                    
            else
                obj.b = double.empty();
                obj.isComplex = false;
            end
            
            % Create symbol table
            obj.SymbolTable = obj.makeTable();
        end
        
        function val = Value(obj, thing)
            arguments 
                obj (1,1) SolvedMomentMatrix
                thing (1,1) RealObject
            end
            
            % Check coefficients exist
            coefs = thing.Coefficients;
            if isempty(coefs)
                error("Could not obtain real coefficients associated with input.");
            end

            % Contract
            val = coefs * obj.a;
        end
    end
    
    methods(Access = private)
        function out_table = makeTable(obj)
            % Copy table, adding values column:
            out_table = obj.MomentMatrix.MatrixSystem.SymbolTable;
            z = num2cell(zeros(1,length(out_table)));
            [out_table.real_value] = z{:};
            [out_table.imaginary_value] = z{:};
            
            % Blit in values
            for index=2:length(out_table)
                if out_table(index).basis_re > 0
                    out_table(index).real_value = ...
                        obj.a(out_table(index).basis_re);
                end
                if obj.isComplex && (out_table(index).basis_im > 0)
                    out_table(index).imaginary_value = ...
                        obj.b(out_table(index).basis_im);
                end
            end            
        end
        
        function val = extract_yalmip_value(obj, item)
            arguments
                obj (1,1) SolvedMomentMatrix
                item (:,:)
            end
            if ~isa(item, 'sdpvar')
                error('Input should be sdpvar.');
            end
            
            val = value(item);
            
            if sum(isnan(val(:))) > 0
                error("NaN returned when retrieving value of item. Has"...
                      + " the SDP yet been solved?");
            end
        end
    end
end

%% Checkers
function isNumericOrSDPvar(item)
    if ~isnumeric(item) && ~isa(item, 'sdpvar')
        error('Input must be numeric, or a yalmip sdpvar.');
    end
end
