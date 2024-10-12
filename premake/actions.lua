function getOS()
	-- ask LuaJIT first
	if jit then
		return jit.os
	end

	-- Unix, Linux variants
	local fh,err = assert(io.popen("uname -o 2>/dev/null","r"))
	if fh then
		osname = fh:read()
	end

	return osname or "Windows"
end

newaction {
    trigger = "format",
    description = "Formats all code files to comply with formatting spec",
    execute = function()
        -- print(os.execute("pwd"))
        local seperator = package.config:sub(1,1);
        -- print("String" .. "Concat")
        -- print(debug.getinfo(1).source)
        local currentluasourcefile = debug.getinfo(1).source;
        local pwd = currentluasourcefile:sub(2, string.len(currentluasourcefile) - 20);
        print(pwd)
        local src = io.popen([[pwd .. "/src"]]):lines();
        print(src)
        for dir in io.popen(pwd .. "/src"):lines() do print(dir) end;
        print(getOS())
        print(getOS():contains("Linux"))
        package.path = package.path .. ";" .. pwd .. seperator .. "premake" .. seperator .. "?.lua"
            .. ";" .. pwd .. seperator .. "premake" .. seperator .. "?" .. seperator .. "init.lua";
        print(package.path)
        local dir = require "pl.dir"
        print(dir.getallfiles())
    end
}
