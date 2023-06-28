function mm_out = MakeMomentMatrix(obj, depth)
    arguments
        obj (1,1) MTKScenario
        depth (1,1) uint64 {mustBeInteger, mustBeNonnegative}
    end

    % Construct matrix
    mm_out = OpMatrix.MomentMatrix(obj.System(), depth);

    % Do binding...
    obj.onNewMomentMatrix(mm_out)
 end