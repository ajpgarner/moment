classdef SymmetrizedMatrixSystem < MatrixSystem
   
    %% Constructor
    methods
        function obj = SymmetrizedMatrixSystem(symSetting)            
            % Superclass c'tor
            obj = obj@MatrixSystem(symSetting);            
        end
    end               
end



