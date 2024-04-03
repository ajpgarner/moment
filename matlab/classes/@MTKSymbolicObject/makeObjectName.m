 function str = makeObjectName(obj)
     % Query MTK for canonical object name(s)
    str = mtk('import_polynomial', 'output', 'string', ...
              obj.Scenario.System.RefId, obj.SymbolCell);
end