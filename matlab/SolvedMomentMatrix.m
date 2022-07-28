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
                real_sol (:,1) double
                im_sol (:,1) double                
            end 
            obj.MomentMatrix = moment_matrix;
            
            % Check and copy real solution
            if (moment_matrix.RealBasisSize ~= length(real_sol))
                error("Real solution doesn't match basis size.");
            end
            obj.a = real_sol;
            
            % Check and copy imaginary solutions, if set
            if (nargin >= 3)
                if (moment_matrix.ImaginaryBasisSize ~= length(im_sol))
                    error("Imaginary solution doesn't match basis size.");
                end            
                obj.b = im_sol;
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
            out_table = obj.MomentMatrix.SymbolTable;
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
                        + obj.b(out_table(index).basis_im);
                end
            end            
        end
    end
end

