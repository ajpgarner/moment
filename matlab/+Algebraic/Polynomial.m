
classdef Polynomial < ComplexObject
    %POLYNOMIAL
    
    properties
        Constituents = Algebraic.Monomial.empty(1,0)
    end
    
    methods
        function obj = Polynomial(setting, constituents)
            arguments
                setting (1,1) Scenario
                constituents (1,:) Algebraic.Monomial
            end
            obj = obj@ComplexObject(setting);
            obj.Constituents = constituents;
            obj.orderAndMerge();
        end
    end
    
    %% Localizing matrix...
    methods
        function val = LocalizingMatrix(obj, level)
            arguments
                obj (1,1) Algebraic.Polynomial
                level (1,1) uint64
            end
            lm = LocalizingMatrix.empty(1,0);
            w = double.empty(1,0);
            
            for c = obj.Constituents
                lm(end+1) = c.RawLocalizingMatrix(level);
                w(end+1) = c.Coefficient;
            end
            
            val = CompositeOperatorMatrix(lm, w);
        end
    end
    
    %% Sums and multiplication
    methods
        % Multiplication
        function val = mtimes(lhs, rhs)
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Pre-multiplication by a built-in type
            if ~isa(lhs, 'Algebraic.Polynomial')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            if isnumeric(other)
                if length(other) ~= 1
                    error("_*_ only supported for scalar multiplication.");
                end
                
                new_coefs = Algebraic.Monomial.empty(1,0);
                for i = 1:length(this.Constituents)
                    old_m = this.Constituents(i);
                    new_coefs(end+1) = Algebraic.Monomial(this.Setting, ...
                        old_m.Operators, ...
                        double(old_m.Coefficient * other));
                end
                val = Algebraic.Polynomial(this.Setting, new_coefs);
            else
                error("_*_ not defined between " + class(lhs) ...
                    + " and " + class(rhs));
            end
        end
        
        % Addition
        function val = plus(lhs, rhs)
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Add a scalar by a built-in type?
            if ~isa(lhs, 'Algebraic.Polynomial')
                if ~isnumeric(lhs)
                    error("_+_ not defined between " + class(lhs) ...
                        + " and " + class(rhs));
                end
                this = rhs;
                other = Algebraic.Monomial(this.Setting, [], double(lhs));
            elseif isa(lhs, 'Algebraic.Polynomial')
                this = lhs;
                other = rhs;
            end
            
            % Add monomial to polynomial?
            if isa(other, 'Algebraic.Monomial')
                if (this.Setting ~= other.Setting)
                    error(this.err_mismatched_scenario);
                end
                components = horzcat(this.Constituents, other);
                val = Algebraic.Polynomial(this.Setting, components);
            elseif isa(other, 'Algebraic.Polynomial')
                if (this.Setting ~= other.Setting)
                    error(this.err_mismatched_scenario);
                end
                components = horzcat(this.Constituents, other.Constituents);
                val = Algebraic.Polynomial(this.Setting, components);
            else
                error("_+_ not defined between " + class(lhs) ...
                    + " and " + class(rhs));
            end
        end
        
        % Substraction
        function val = minus(lhs, rhs)
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            val = lhs + -rhs;            
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function success = calculateCoefficients(obj)
            % Early exit if we can't get symbol information for all parts...
            if ~all([obj.Constituents.FoundSymbol])
                success = false;
                return;
            end
            sys = obj.Setting.System;
            
            % Real co-efficients
            obj.real_coefs = sparse(1, sys.RealVarCount);
            obj.im_coefs = sparse(1, sys.ImaginaryVarCount);
            
            for index = 1:length(obj.Constituents)
                cObj = obj.Constituents(index);
                cObj.refreshCoefficients();
                obj.real_coefs = obj.real_coefs + cObj.real_coefs;
                obj.im_coefs = obj.im_coefs + cObj.im_coefs;
            end
            success = true;
        end
    end
    
    
    %% Internal methods
    methods(Access=private)
        function orderAndMerge(obj)
            [~, order] = sort([obj.Constituents.Hash]);
            write_index = 0;
            
            nc = Algebraic.Monomial.empty(1,0);
            last_hash = 0;
            for i = 1:length(obj.Constituents)
                cObj = obj.Constituents(order(i));
                if last_hash == cObj.Hash
                    ncoef = nc(write_index).Coefficient + cObj.Coefficient;
                    nc(write_index) = ...
                        Algebraic.Monomial(obj.Setting, ...
                        cObj.Operators, ...
                        ncoef);
                else
                    write_index = write_index + 1;
                    nc(end+1) = Algebraic.Monomial(obj.Setting, ...
                        cObj.Operators, ...
                        cObj.Coefficient);
                end
                last_hash = cObj.Hash;
            end
            obj.Constituents = nc;
        end
    end
end

