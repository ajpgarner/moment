function val = mtk_locality_format(new_val)
% MTK_LOCALITY_FORMAT Gets/set global "locality format" setting.
% Current options are Natural and Traditional.
%
% Example: Party B's operator 3 associated with measurement x.
%       Natural: 'B.x3'
%   Traditional: 'B3|x'
%
    if nargin == 0        
        s = mtk('settings', 'structured');
        val = s.locality_format;
        return;
    end
    s = mtk('settings', 'structured', 'locality_format', new_val);
    val = s.locality_format;
end

