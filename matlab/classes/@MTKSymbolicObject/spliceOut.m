 function spliceOut(output, source, indices)
 
    spliceOut@MTKObject(output, source, indices);

    output.SymbolCell = source.SymbolCell(indices{:});    
end