classdef ComplexObject < handle
    %COMPLEXOBJECT 
        
    properties(GetAccess = public, SetAccess = private)
        Setting
    end
    
    properties(Dependent, GetAccess = public, SetAccess = private)
        RealCoefficients
        ImaginaryCoefficients
    end
    
    properties(Access=protected)
        real_coefs = sparse(1,0)
        im_coefs = sparse(1,0)
    end    
    
    properties(Access = private)
        has_coefs = false
    end
    
    properties(Constant, Access = protected)
        err_mismatched_scenario = ...
            'Cannot combine objects from different settings.';
    end
    
    
    %% Constructor
    methods
        function obj = ComplexObject(setting)
            arguments
                setting (1,1) Scenario
            end
            obj.Setting = setting;
        end
    end
    
    %% Co-efficients, and accessors
    methods
        function val = get.RealCoefficients(obj)
            if ~obj.has_coefs
                success = obj.calculateCoefficients();
                if success
                    obj.has_coefs = true;
                else
                    val = false;
                    return;
                end
            end
            val = obj.real_coefs;
        end
        
        function val = get.ImaginaryCoefficients(obj)
            if ~obj.has_coefs
                success = obj.calculateCoefficients();
                if success
                    obj.has_coefs = true;
                else
                    val = false;
                    return;
                end
            end
            val = obj.im_coefs;
        end
        
        function success = refreshCoefficients(obj)
            success = obj.calculateCoefficients();
            if success
                obj.has_coefs = true;
            end
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function success = calculateCoefficients(obj)
           success = false;
        end
    end
end

