classdef CollinsGisin
    %COLLINSGISIN Summary of this class goes here
    %   Detailed explanation goes here
   
    properties(SetAccess={?CollinsGisin, ?Scenario}, GetAccess=public)
        Scenario
        Shape
        Sequences
        Symbols
        BasisElements
    end
    
    properties(Access=private)
        mono_coefs = double.empty
        cache_seq = string.empty
        cache_sym = uint64.empty
        cache_basis = uint64.empty
    end
    
    methods
        function obj = CollinsGisin(scenario)
            arguments
                scenario (1,1) Scenario
            end
            obj.Scenario = scenario;
            obj.Shape = obj.Scenario.OperatorsPerParty + 1;
        end
        
        function val = get.Sequences(obj)
            if isempty(obj.cache_seq)         
                obj.cache_seq = npatk('collins_gisin', 'sequences', ...
                                      obj.Scenario.GetMatrixSystem.RefId);
            end
            val = obj.cache_seq;
        end
       
        function val = get.Symbols(obj)
            if isempty(obj.cache_seq)         
                obj.cache_sym = npatk('collins_gisin', 'symbols', ...
                                      obj.Scenario.GetMatrixSystem.RefId);
            end
            val = obj.cache_sym;
        end
        
        function val = get.BasisElements(obj)
            if isempty(obj.cache_basis)         
                obj.cache_basis = npatk('collins_gisin', 'basis', ...
                                       obj.Scenario.GetMatrixSystem.RefId);
            end
            val = obj.cache_basis;
        end
        
        function val = linfunc(obj, tensor)
             arguments
                obj (1,1) Scenario.CollinsGisin
                tensor double
            end
            if (length(size(tensor)) ~= length(obj.Shape)) ...
                || any(size(tensor) ~= obj.Shape)
                error("Expected tensor with dimension " + ...
                    mat2str(obj.Shape));
            end
            
            total_size = prod(obj.Shape);
            the_basis = obj.BasisElements;
            
            non_triv = nnz(tensor);
            
            sparse_i = ones(1, non_triv);
            sparse_j = zeros(1, non_triv);
            sparse_val = zeros(1, non_triv);
            
            write_index = 1;
            for i = 1:total_size
                if tensor(i) ~= 0
                    sparse_j(write_index) = the_basis(i);
                    sparse_val(write_index) = tensor(i);
                    write_index = write_index + 1;
                end
            end
            
            real_coefs = sparse(sparse_i, sparse_j, sparse_val, ...
                                1, obj.Scenario.GetMatrixSystem.RealVarCount);
           
            val = RealObject(obj.Scenario, real_coefs);
        end
    end
end

