classdef ImportedScenario < Abstract.Scenario
    %IMPORTEDSCENARIO Manually input matrices
    %    
    properties(GetAccess = public, SetAccess = protected)
        Real
    end
        
    %% Construction and initialization
    methods
        function obj = ImportedScenario(all_real)
            arguments
                all_real (1,1) logical = false
            end
            
            % Superclass c'tor
            obj = obj@Abstract.Scenario();
            
            % Set whether all symbols are real or not
            obj.Real = logical(all_real);            
        end
        
        
        function val = Clone(obj)
            % Copy scenario
            arguments
                obj (1,1) InflationScenario
            end
            
            % Clone ImportedScenario
            val = ImportedScenario(obj.Real);
        end
    end
    
    %% Virtual methods
    methods(Access={?Abstract.Scenario,?MatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            arguments
                obj (1,1) ImportedScenario
            end
            cell_args = cell.empty;
            if obj.Real
                cell_args{end+1} = 'real';
            end
            
            ref_id = mtk('new_imported_matrix_system', cell_args{:});
        end
    end
    
    %% Input methods
    methods
        function val = ImportMatrix(obj, input, matrix_type)
            arguments
                obj (1,1) ImportedScenario
                input
                matrix_type 
            end
            cell_args = cell.empty;
            
            % Validate input
            if nargin < 2
               error("A matrix must be supplied as input.");
            end
            if ~ismatrix(input)
                error("Input must be a matrix");
            end
            [dimension, other_dimension] = size(input);
            if other_dimension ~= dimension
                error("Input must be a square matrix.");
            end
            
            % Validate matrix type
            if nargin >= 3
                matrix_type = lower(char(matrix_type));
                if ~ismember(matrix_type, ...
                            {'real', 'complex', 'symmetric', 'hermitian'})
                    error("Matrix type must be real, complex, symmetric or hermitian.");
                end
                cell_args{end+1} = matrix_type;
                
            end
            index = mtk('import_matrix', obj.System.RefId, ...
                                     input, cell_args{:});

            val = OpMatrix.OperatorMatrix(obj.System, index, dimension);
            
            % Update symbols, with forced reset
            obj.System.UpdateSymbolTable(true);            
        end
        
        function val = ImportSymmetricMatrix(obj, input)
            arguments
                obj (1,1) ImportedScenario
                input
            end
            val = obj.ImportMatrix(input, 'symmetric');
        end
        
        function val = ImportHermitianMatrix(obj, input)
            arguments
                obj (1,1) ImportedScenario
                input
            end
            val = obj.ImportMatrix(input, 'hermitian');
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) ImportedScenario
                mm (1,1) OpMatrix.MomentMatrix
            end
            error('Imported scenario can not generate matrices.');
        end
    end
end

