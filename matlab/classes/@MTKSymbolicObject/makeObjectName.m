 function str = makeObjectName(obj)
     % Query MTK for canonical object name(s)
    str = mtk('simplify', 'polynomial', 'string_out', ...
              obj.Scenario.System.RefId, obj.SymbolCell);
end