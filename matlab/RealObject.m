classdef RealObject < handle
    %REALOBJECT Object linked to moment matrix real-coefficients.
    
    %% Properties
    properties(SetAccess=private, GetAccess=public)
        Coefficients
        Scenario
    end
    
    properties(Access=protected)
        real_coefs
        matrix_system
    end
    
    properties(Constant, Access = protected)
        err_no_matrix_system = ...
            ['Coefficients can not be calculated. This is because a ', ...
            'MatrixSystem has not yet been associated with this ',...
            'setting.'];
        
        err_mismatched_scenario = ...
            'Cannot combine objects from different settings.';
    end
    
    %% Public methods
    methods
        function obj = RealObject(setting, coefs)
            %REALOBJECT Construct an instance of this class
            obj.Scenario = setting;
            obj.matrix_system = MatrixSystem.empty;
            if nargin >= 2
                obj.real_coefs = coefs;
            end
        end
        
        function val = get.Coefficients(obj)
            if isempty(obj.real_coefs)
                % (Silently) do nothing if no moment matrix yet defined.
                if ~obj.Scenario.HasMatrixSystem
                    val = double.empty;
                    return
                else 
                    % Otherwise, bind scenario's matrix system to this.
                    obj.matrix_system = obj.Scenario.System();
                end
                
                % Call do-function
                obj.calculateCoefficients();
            end
            
            % Check co-efficient length, and pad if necessary
            obj.padCoefficients();
            
            % Return co-efficients
            val = obj.real_coefs;
        end
        
        function val = RebuildCoefficients(obj)
            % Forced rebuilding of co-efficients
            
            % Require moment-matrix to exist
            if ~obj.Scenario.HasMatrixSystem
                error(obj.err_no_matrix_system);
            else
                obj.matrix_system = obj.Scenario.System();
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

                val = RealObject(this.Scenario);
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
            if lhs.Scenario ~= rhs.Scenario
                error(obj.err_mismatched_scenario);
            end
            
            lhs_coefs = lhs.getCoefficientsOrFail();
            rhs_coefs = rhs.getCoefficientsOrFail();
            
            % Build added real object
            val = RealObject(lhs.Scenario);
            val.real_coefs = lhs_coefs + rhs_coefs;
        end
        
        function val = minus(lhs, rhs)
            arguments
                lhs (1,1) RealObject
                rhs (1,1) RealObject
            end
            if lhs.Scenario ~= rhs.Scenario
                error(obj.err_mismatched_scenario);
            end
            
            lhs_coefs = lhs.getCoefficientsOrFail();
            rhs_coefs = rhs.getCoefficientsOrFail();
            
            % Build subtracted real object
            val = RealObject(lhs.Scenario);
            val.real_coefs = lhs_coefs - rhs_coefs;
        end
        
        function val = uminus(obj)
            arguments
                obj (1,1) RealObject
            end
            coefs = obj.getCoefficientsOrFail();
            
            % Build negated real object
            val = RealObject(obj.Scenario);
            val.real_coefs = -coefs;
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
                error("CVX real basis vector dimension (" ...
                    + num2str(length(real_basis)) + ") does not match "...
                    + "object coefficient dimension (" ...
                    + num2str(length(coefs)) + ").");
            end
            
            % Generate expression...
            cvx_expr = coefs * real_basis;
        end
    end
    
    %% Public yalmip methods
     methods
        function ym_expr = yalmip(obj, real_basis)
            arguments
                obj (1,1) RealObject
                real_basis (:, 1)
            end
            % Get coefficients
            coefs = obj.getCoefficientsOrFail();
            
            % real_basis should be sdpvar object
            if ~isa(real_basis, 'sdpvar')
                error("Expected yalmip real basis vector input.");
            end
            
            if length(coefs) ~= length(real_basis)
                error("Yalmip sdpvar vector dimension does not match "...
                    + "object coefficient dimension.");
            end
            
            % Generate expression...
            ym_expr = coefs * real_basis;
        end
    end
    
    %% Protected methods
    methods(Access={?RealObject,?LocalityScenario})
        function setCoefficients(obj, coefs)
            obj.real_coefs = coefs;
        end
        
        function coefs = getCoefficientsOrFail(obj)
            coefs = obj.Coefficients;
            
            % NB: Zero sparse array is not empty...!
            if isempty(coefs)
                error(obj.err_no_matrix_system);
            end
        end
        
        function padCoefficients(obj)
            % Do nothing, if no matrix system
            if isempty(obj.matrix_system)
                return
            end
            
            % How long should co-efficients be
            desired_length = obj.matrix_system.RealVarCount;
            
            % Matrix of zeros, if empty
            if isempty(obj.real_coefs)
                obj.real_coefs = sparse(zeros(1, desired_length));    
                return
            end
            
            % Pad if not long enough
            if length(obj.real_coefs) ~= desired_length
                excess = double(desired_length - length(obj.real_coefs));
                if excess < 0
                    error("Co-efficients should not shrink!");
                end
                
                obj.real_coefs = ...
                    sparse(padarray(obj.real_coefs, [0, excess], 'post'));
            end               
            
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function calculateCoefficients(obj)
            % Overload this!
            obj.real_coefs = ...
                sparse(zeros(1, obj.matrix_system.RealVarCount));
        end
    end
    
end

