classdef SolvedScenario < handle
    %SOLVEDSCENARIO Feed results of an SDP solve back into a scenario
    
    properties(SetAccess=private, GetAccess=public)
        Scenario
        RealValues
        ImaginaryValues
        SymbolTable
    end
    
    properties(Dependent, SetAccess=private, GetAccess=public)
        IsComplex
    end
    
    %% Constructor
    methods
        function obj = SolvedScenario(setting, real_vals, im_vals)
            %SOLVEDSETTING Construct an instance of this class.
            % From either (Scenario, SolvedMomentMatrix) or
            % (Scenario, MomentMatrix, Symmetric elements, Anti-sym elements).
            arguments
                setting (1,1) MTKScenario
                real_vals (:,1) = double.empty(0,1);
                im_vals (:,1) = double.empty(0,1);
            end
            
            % Save handle
            obj.Scenario = setting;
            
            % Get real numbers (pad with NaN if necessary)
            expected_real = obj.Scenario.System.RealVarCount;
            if isa(real_vals, 'sdpvar')
                real_vals = value(real_vals);
            end                
            delta_re = length(real_vals) - expected_real;
            if delta_re > 0
                real_vals = [real_vals, NaN(1, delta_re)];
            end
            obj.RealValues = real_vals;
            
            % Get imaginary numbers (pad with NaN if necessary)
            expected_im = obj.Scenario.System.ImaginaryVarCount;
            if isa(im_vals, 'sdpvar')
                im_vals = value(im_vals);
            end
            delta_im = length(im_vals) - expected_im;
            if delta_im > 0
                im_vals = [im_vals, NaN(1, delta_im)];
            end
            obj.ImaginaryValues = im_vals;
            
            % Build symbol table
            obj.buildSymbolTable();
        end
    end
    
    %% Dependent variables
    methods
        function val = get.IsComplex(obj)
            val = logical(~isempty(obj.ImaginaryValues));
        end
    end
    
    %% Bind solution to scenario objects
    methods
                       
        function val = Value(obj, thing)
            arguments 
                obj (1,1) SolvedScenario.SolvedScenario
                thing (1,1) 
            end
            
            if isa(thing, 'Abstract.RealObject')  
                obj.checkScenario(thing)
                val = thing.Apply(obj.RealValues);                
            elseif isa(thing, 'MTKObject')
                obj.checkScenario(thing)
                val = thing.Apply(obj.RealValues, obj.ImaginaryValues);                
            else
                error("Unable to assign a value to object of type %s",...
                      class(thing));
            end
        end
    end
    
    methods(Access=protected)
        function checkScenario(obj, what)
            if what.Scenario ~= obj.Scenario
                error("Scenario of input must match SolvedScenario's.");
            end
        end
    end
    
    methods(Access=private)
        function buildSymbolTable(obj)
            % Copy table, adding column for values:
            obj.SymbolTable = obj.Scenario.System.SymbolTable;
           
            if obj.IsComplex
                [obj.SymbolTable.value] = deal(complex(NaN));
            else
                [obj.SymbolTable.value] = deal(double(NaN));
            end
            
            for index=1:length(obj.SymbolTable)
                b_re = obj.SymbolTable(index).basis_re;
                assert(b_re <= length(obj.RealValues), ...
                       "Symbol table refers to real basis element not in solution.");                                   
                if b_re > 0
                    val = obj.RealValues(b_re);
                else
                    val = 0;
                end
                
                if obj.IsComplex 
                    b_im = obj.SymbolTable(index).basis_im;
                    assert(b_im <= length(obj.ImaginaryValues), ...
                       "Symbol table refers to imaginary basis element not in solution.");
                    if b_im > 0
                        val = val + 1i * obj.ImaginaryValues(b_im);
                    end
                end                
                obj.SymbolTable(index).value = val;
            end
        end
    end
end

