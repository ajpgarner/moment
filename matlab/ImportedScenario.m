classdef ImportedScenario < Scenario
    %IMPORTEDSCENARIO Manually input matrices
    %    
    properties(GetAccess = public, SetAccess = protected)
        Real
    end
    
    
    %% Construction and initialization
    methods
        function obj = ImportedScenario(all_real)
            % Superclass c'tor
            obj = obj@Scenario();
            
            % Set inflation level
            if nargin>=1
                obj.Real = logical(all_real);
            else 
                obj.Real = false;
            end            
        end
        function val = Clone(obj)
            arguments
                obj (1,1) InflationScenario
            end
            
            % Clone ImportedScenario
            val = ImportedScenario(obj.Real);
        end
        
    end
    
    %% Friend/interface methods
    methods(Access={?Scenario,?MatrixSystem})
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
            cell_args = cell.empty;
            
            % Validate input
            if nargin < 2
               error("A matrix must be supplied as input.");
            end
            if ndims(input)~=2
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

            val = OperatorMatrix(obj.System, index, dimension);
            
            % Update symbols, with forced reset
            obj.System.UpdateSymbolTable(true);            
        end
        
        function val = ImportSymmetricMatrix(obj, input)
            val = obj.ImportMatrix(input, 'symmetric');
        end
        
        function val = ImportHermitianMatrix(obj, input)
            val = obj.ImportMatrix(input, 'hermitian');
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) ImportedScenario
                mm (1,1) MomentMatrix
            end
            error('Imported scenario can not generate matrices.');
        end
    end
end

