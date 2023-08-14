classdef MTKMomentRulebook < handle
%MTKMOMENTRULEBOOK A list of substitutions to make at the level of moments.
%

%% Properties
properties(GetAccess = public, SetAccess = protected)
    Scenario % Associated scenario.
    Id       % ID of the rulebook within the matrix system.
end

properties(Dependent, GetAccess = public, SetAccess = private)
    SymbolCell      % Rules as cell array of symbol substitutions.
    Polynomials     % Rules as polynomials.
    Strings         % Rules as strings    
end

properties(Access=private)
    cache_symbol_cell
    cache_polys
    cache_strs;
    has_symbol_cell = false;
    has_polys = false;
    has_strs = false;
end

properties(Constant, Access = private)
    err_locked = ['No more changes to this rulebook are possible, ',...
                  'because it has already been applied to a matrix.'];
end
    

%% Constructor
methods
    function obj = MTKMomentRulebook(scenario, label, varargin)
    % MOMENTRULEBOOK Creates a moment substitution rule book.                
        if nargin < 1 || ~isa(scenario,'MTKScenario')
            error("First argument must be a MTKScenario.");
        end
        
        if nargin < 2
            label = "";
        else
            label = string(label);
        end
        
        obj.Scenario = scenario;

        % Extra arguments to MTK
        rb_args = cell(1,0);       
        if ~isempty(label) && (label ~= "")
            rb_args{end+1} = "label";
            rb_args{end+1} = label;
        end
        
        % Create rulebook with no rules
        obj.Id = mtk('create_moment_rules', rb_args{:}, ...
                             scenario.System.RefId, {});
        obj.invalidate_cached_rules();
        
        % Add any initial rules...
        for i = 1:numel(varargin)
            obj.Add(varargin{i});
        end        
    end
end

%% Getter methods
methods
    function val = get.SymbolCell(obj)
        if ~obj.has_symbol_cell
            obj.cache_symbol_cell = ...
                mtk('moment_rules', obj.Scenario.System.RefId, ...
                    obj.Id, 'symbols');
            obj.has_symbol_cell = true;
        end
        val = obj.cache_symbol_cell;
    end
    
    function val = get.Polynomials(obj)
        if ~obj.has_polys            
            poly_cell = mtk('moment_rules', obj.Scenario.System.RefId, ...
                            obj.Id, 'polynomials');
            obj.cache_polys = MTKPolynomial.InitFromOperatorPolySpec(...
                                obj.Scenario, poly_cell);
            obj.has_polys = true;
        end
        val = obj.cache_polys;
    end
    
    function val = get.Strings(obj)
        if ~obj.has_strs
            obj.cache_strs = ...
                mtk('moment_rules', obj.Scenario.System.RefId, ...
                    obj.Id, 'strings');
            obj.has_strs = true;
        end
        val = obj.cache_strs;
    end
    
    function invalidate_cached_rules(obj)
         obj.cache_symbol_cell = cell.empty(0, 1);
         obj.cache_polys = MTKPolynomial.empty(0, 1);
         obj.cache_strs = string.empty(0, 1);
         obj.has_symbol_cell = false;
         obj.has_polys = false;
         obj.cache_strs = false;
    end
end


%% Add rules
methods
    function Add(obj, new_rules, new_symbols)
    % ADD Adds rules based on (list of) polynomials that should be zero.
    %
    % PARAMS
    %   new_rules - One or more polynomial rules.
    %   new_symbols - Set to true to create symbols in setting if they do
    %                 not already exist.
    %
        if nargin < 2
            error("Add requires at least one parameter.");
        end
        if nargin < 3
            new_symbols = true;
        end
        
        if isa(new_rules, 'MTKPolynomial')
            obj.AddFromPolynomial(new_rules, new_symbols);
        elseif iscell(new_rules)
            obj.AddFromSymbolCell(new_rules);
        else
            error("Cannot add rules of type %s.", class(new_rules));
        end

    end
    
    
    function AddFromPolynomial(obj, new_rules, new_symbols)
    % FROMOPERATORSEQUENCES Creates from operator sequence cell array.
    %
    % Provide a cell array of polynomials; each polynomial represented as a
    % cell array of cell-array pairs {[operators], factor}.
    %
    % If new_symbols is true, then register any 'missing' operator
    % sequences. Otherwise, an error will be thrown if an unrecognised
    % sequence is supplied.
    %
    
        % Parse inputs
        assert((nargin >= 2) && isa(new_rules, 'MTKPolynomial'));
        if nargin < 3
            new_symbols = false;
        end
        
        if new_symbols
            % Prepare operator cell
            op_cells = new_rules.OperatorCell;
            obj.AddFromOperatorCell(op_cells, new_symbols);        
        else
            % Prepare symbol cell
            try
				sym_cell = new_rules.SymbolCell;
            catch Exception
                error("Cannot create rules from polynomials: %s", ...
                      Exception.message);
            end
            
            obj.AddFromSymbolCell(sym_cell)
        end
    end
    
    function AddFromOperatorCell(obj, op_seq_cell, new_symbols)
    % FROMOPERATORSEQUENCES Creates from operator sequence cell array.
    %
    % Provide a cell array of polynomials; each polynomial represented as a
    % cell array of cell-array pairs {[operators], factor}.
    %
    % If new_symbols is true, then register any 'missing' operator
    % sequences. Otherwise, an error will be thrown if an unrecognised
    % sequence is supplied.
    %

        % Parse inputs
        assert((nargin >= 2) && iscell(op_seq_cell));
        if nargin < 3
            new_symbols = false;
        end
        
        % Prepare params
        extra_params = {'input', 'sequences', 'rulebook', obj.Id};
        if ~new_symbols
            extra_params{end+1} = 'no_new_symbols';
        end

        % Construct rulebook, and import rules
        rule_id = mtk('create_moment_rules', extra_params{:},...
                      obj.Scenario.System.RefId, op_seq_cell);
        assert(rule_id == obj.Id);
        
        obj.invalidate_cached_rules();
        
        % Flag to system object that extra symbols may have been created.
        if new_symbols
            obj.Scenario.System.UpdateSymbolTable();
        end        
    end
    
    function AddFromSymbolCell(obj, symbol_cell)
    % FROMOPERATORSEQUENCES Creates from operator sequence cell array.
    %
    % Provide a cell array of polynomials; each polynomial represented as a
    % cell array of cell-array pairs {[operators], factor}.
    %
    % If new_symbols is true, then register any 'missing' operator
    % sequences. Otherwise, an error will be thrown if an unrecognised
    % sequence is supplied.
    %

        % Validate inputs
        assert((nargin == 2) && iscell(symbol_cell));

        % Construct rulebook, and import rules
        rule_id = mtk('create_moment_rules', 'input', 'symbols', ...
                      'rulebook', obj.Id, 'no_new_symbols', ...
                      obj.Scenario.System.RefId, symbol_cell);
        assert(rule_id == obj.Id);
        
        obj.invalidate_cached_rules();     
    end
    
    function AddFromList(obj, varargin)
    % ADDFROMLIST Add replacement rules, each symbol by a scalar.
    %
    % SYNTAX:
    %   1. obj.AddFromList([cell array])
    %   2. obj.AddFromList([symbol_id_array], [value_array])
    %
    % For syntax 1, cell array should consist of pairs {symbol_id, value}.
    % For syntax 2, two arrays should have same size.
    %
    
        if nargin == 2
            cell_input = varargin{1};
        elseif narargin == 3
            % Parse inputs:
            symbol_ids = reshape(uint64(varargin{1}), [], 1);
            values = reshape(double(varargin{2}), [], 1);        
            if numel(symbol_ids) ~= numel(values)
                error("Number of symbol IDs should match number of values.");
            end

            % Make substitution cell input
            cell_input = cell(length(symbol_ids), 1);
            for idx=1:length(symbol_ids)
                cell_input{idx} = {uint64(symbol_ids(idx)), ...
                                   double(values(idx))};
            end
        else
            error("AddScalarSubstitutionList requires one or two arguments.");
        end
            

        % Call rulebook
        rule_id = mtk('create_moment_rules', 'input', 'list', ...
                      'rulebook', obj.Id, 'no_new_symbols', ...
                      obj.Scenario.System.RefId, cell_input);
        assert(rule_id == obj.Id);
     
        obj.invalidate_cached_rules();
    end
end

%% Apply rules to objects
methods
    function val = Apply(obj, target)
        if nargin < 2
            error("Missing argument: target for rules to be applied to.");
        end
        
        if isa(target, 'MTKObject')
            val = target.ApplyRules(obj);
            return;
        end
        
        error("Could not apply rules to target of type %s.", ...
              class(target));        
    end
end
    
end
    
