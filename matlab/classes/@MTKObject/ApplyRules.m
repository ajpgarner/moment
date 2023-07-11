function val = ApplyRules(obj, rulebook)
%APPLYRULES Apply rules to this MTKObject.
% Should be overloaded by child classes.

    warning("Applying rules to object of type %s has no effect.",...
            class(obj));
   
    val = obj;
end

