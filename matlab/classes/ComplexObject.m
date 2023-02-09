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
                    error("Could not retrieve real coefficients.");
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
                    error("Could not retrieve imaginary coefficients.");
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
    
    %% Public CVX methods
    methods
        function cvx_expr = cvx(obj, real_basis, im_basis)
             % Get coefficients
            the_re_coefs = obj.RealCoefficients;
            the_im_coefs = obj.ImaginaryCoefficients;
            
            % Has real...
            if nargin >= 2
                if ~isa(real_basis, 'cvx')
                    error("Expected CVX real basis vector input.");
                end
                if length(the_re_coefs) ~= length(real_basis)
                    error("CVX real vector dimension (" ...
                        + num2str(length(real_basis)) + ") does not match "...
                        + "object coefficient dimension (" ...
                        + num2str(length(coefs)) + ").");
                end
                real_basis = reshape(real_basis, [], 1);
            else
                error("Expected CVX real basis vector input.");
            end
            
            % Has imaginary...
            if nargin >= 3
                if ~isa(im_basis, 'cvx')
                    error("Expected CVX imaginary basis vector input.");
                end
                if length(the_im_coefs) ~= length(im_basis)
                    error("CVX imaginary vector dimension (" ...
                        + num2str(length(real_basis)) + ") does not match "...
                        + "object coefficient dimension (" ...
                        + num2str(length(coefs)) + ").");
                end
                im_basis = reshape(im_basis, [], 1);
                has_im = true;
            else
                the_im_coefs = zeros(1, length(real_basis));
                has_im = false;
            end
            
            % Generate expression...
            cvx_expr = (the_re_coefs * real_basis);
            if (has_im)
                cvx_expr = cvx_expr + 1i * (the_im_coefs * im_basis);
            end
        end
    end
    
        %% Public yalmip methods
     methods
        function ym_expr = yalmip(obj, real_basis, im_basis)
            % Get coefficients
            the_re_coefs = obj.RealCoefficients;
            the_im_coefs = obj.ImaginaryCoefficients;

            % Has real...
            if nargin >= 2
                if ~isa(real_basis, 'sdpvar')
                    error("Expected YALMIP real basis vector input.");
                end
                if length(the_re_coefs) ~= length(real_basis)
                    error("YALMIP real vector dimension (" ...
                        + num2str(length(real_basis)) + ") does not match "...
                        + "object coefficient dimension (" ...
                        + num2str(length(the_re_coefs)) + ").");
                end
                real_basis = reshape(real_basis, [], 1);
            else
                error("Expected YALMIP real basis vector input.");
            end
            
            % Has imaginary...
            if nargin >= 3
                if ~isa(im_basis, 'sdpvar')
                    error("Expected YALMIP imaginary basis vector input.");
                end
                if length(the_im_coefs) ~= length(im_basis)
                    error("YALMIP imaginary vector dimension (" ...
                        + num2str(length(im_basis)) + ") does not match "...
                        + "object coefficient dimension (" ...
                        + num2str(length(the_im_coefs)) + ").");
                end
                im_basis = reshape(im_basis, [], 1);
                has_im = true;
            else
                the_im_coefs = zeros(1, length(real_basis));
                has_im = false;
            end
           
            % Generate expression...
            ym_expr = (the_re_coefs * real_basis);
            if (has_im)
                ym_expr = ym_expr + 1i * (the_im_coefs * im_basis);
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

