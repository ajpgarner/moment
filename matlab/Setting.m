classdef Setting < handle
    %SETTING A scenario involving multiple agents with measurements.
    %   
      
    properties(GetAccess = public, SetAccess = protected)
        Parties
    end
    
    properties(Access = private)
        moment_matrix
    end
    
    methods
        function obj = Setting(initial_parties)
            arguments
                initial_parties (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            obj.Parties = Party.empty;
            if (initial_parties >=1 )
                for x = 1:initial_parties 
                    obj.Parties(end+1) = Party(x);
                end
            end
        end
        
        function AddParty(obj, name)            
            next_id = length(obj.Parties)+1;
            if nargin >=2
                obj.Parties(end+1) = Party(next_id, name);
            else
                obj.Parties(end+1) = Party(next_id);
            end            
        end
        
        function mm_out = MakeMomentMatrix(obj, depth, skip_bind)
            arguments
                obj 
                depth (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                skip_bind (1,1) logical = false
            end
            obj.moment_matrix = MomentMatrix(obj, depth);          
            if ~skip_bind
                obj.do_bind(obj.moment_matrix)
            end
            mm_out = obj.moment_matrix;
        end
    end
    
    methods(Access=private)
        function do_bind(obj, mm)
             arguments
                obj (1,1) Setting
                mm (1,1) MomentMatrix
             end
             p_table = mm.ProbabilityTable;
             for p_row = p_table
                 
             end
        end
    end
end

