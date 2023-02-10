classdef Normalization < handle & Abstract.RealObject
    %NORMALIZATION The normalization element of the setting (typically 1)
    
    methods
        function obj = Normalization(scenario)
            %NORMALIZATION Construct an instance of this class
    
            % Superclass c'tor
            obj = obj@Abstract.RealObject(scenario);
        end
    end   
end

