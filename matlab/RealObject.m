classdef RealObject < handle
    %REALOBJECT Object linked to moment matrix real-coefficients.
    
    %% Properties
    properties(SetAccess=private, GetAccess=public)
        Coefficients
        Setting
    end
    
    properties(Access=protected)
        real_coefs
    end
    
    %% Public methods
    methods
        function obj = RealObject(setting)
            %REALOBJECT Construct an instance of this class
            obj.Setting = setting;
        end
        
        function val = get.Coefficients(obj)
            if isempty(obj.real_coefs)
                % (Silently) do nothing if no moment matrix yet defined.
                if ~obj.Setting.HasMomentMatrix
                    val = double.empty;
                    return
                end
                
                % Otherwise, call do-function
                obj.calculateCoefficients();
            end
            
            val = obj.real_coefs;
        end
        
        function val = RebuildCoefficients(obj)
            % Forced rebuilding of co-efficients
            
            % Require moment-matrix to exist
            if ~obj.Setting.HasMomentMatrix
                error("No moment matrix has yet been defined for" ...
                    + " this setting.");
            end
            
            % Call do-function, then return coefficients
            obj.calculateCoefficients();
            val = obj.real_coefs;
        end
        
        function val = apply(lhs, rhs)
            arguments
                lhs (1,1) RealObject
                rhs (:,1)
            end
            coefs = lhs.getCoefficientsOrFail();
            if length(coefs) ~= length(rhs)
                error("Basis vector and coefficient length do not match.");
            end
            val = coefs * rhs;
        end
        
        function val = mtimes(lhs, rhs)
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Pre-multiplication by a built-in type (probably...)
            if ~isa(lhs, 'RealObject')
                pre_mult = true;
                coefs = rhs.getCoefficientsOrFail();
                this = rhs;
                other = lhs;
            else 
                pre_mult = false;
                coefs = lhs.getCoefficientsOrFail();
                this = lhs;
                other = rhs;
            end
            
            if isnumeric(other)
                if length(other) ~= 1
                    error("_*_ only supported for scalar multiplication.");
                end

                val = RealObject(this.Setting);
                if pre_mult
                    val.real_coefs = other * coefs;
                else
                    val.real_coefs = coefs * other;
                end
            else
                error("_*_ not defined between " + class(lhs) ...
                    + " and " + class(rhs));
            end
        end
         
        function val = plus(lhs, rhs)
            arguments
                lhs (1,1) RealObject
                rhs (1,1) RealObject
            end
            if lhs.Setting ~= rhs.Setting
                error("Cannot add objects from different settings.");
            end
            
            lhs_coefs = lhs.getCoefficientsOrFail();
            rhs_coefs = rhs.getCoefficientsOrFail();
            
            % Build added real object
            val = RealObject(lhs.Setting);
            val.real_coefs = lhs_coefs + rhs_coefs;
        end
        
        function val = minus(lhs, rhs)
            arguments
                lhs (1,1) RealObject
                rhs (1,1) RealObject
            end
            if lhs.Setting ~= rhs.Setting
                error("Cannot add objects from different settings.");
            end
            
            lhs_coefs = lhs.getCoefficientsOrFail();
            rhs_coefs = rhs.getCoefficientsOrFail();
            
            % Build subtracted real object
            val = RealObject(lhs.Setting);
            val.real_coefs = lhs_coefs - rhs_coefs;
        end
    end
    
    
    %% Public CVX methods
    methods
        function cvx_expr = cvx(obj, real_basis)
            arguments
                obj (1,1) RealObject
                real_basis (:, 1)
            end
            % Get coefficients
            coefs = obj.getCoefficientsOrFail();
            
            % real_basis should be cvx object
            if ~isa(real_basis, 'cvx')
                error("Expected CVX real basis vector input.");
            end
            
            if length(coefs) ~= length(real_basis)
                error("CVX real basis vector dimension does not match "...
                    + "Correlator coefficient dimension.");
            end
            
            % Generate expression...
            cvx_expr = coefs * real_basis;
        end
    end
    
    %% Protected methods
    methods(Access={?RealObject,?Setting})
        function setCoefficients(obj, coefs)
            obj.real_coefs = coefs;
        end
        
        function coefs = getCoefficientsOrFail(obj)
            coefs = obj.Coefficients;
            % NB: Zero sparse array is not empty...!
            if isempty(coefs)
                error("Coefficients can not be calculated. Perhaps a "...
                    + "MomentMatrix has not yet been generated for "...
                    + "this setting?");
            end
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function calculateCoefficients(obj)
            % Overload this!
        end
    end
    
end

