classdef Setting < handle
    %SETTING A scenario involving multiple agents with measurements.
    %   
      
    properties(GetAccess = public, SetAccess = protected)
        Parties
    end
    
    methods
        function obj = Setting(initial_parties)
            obj.Parties = Party.empty;
            initial_parties = uint64(initial_parties);
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
    end
end

