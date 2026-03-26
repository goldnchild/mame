-- license:MIT
-- copyright-holders:Carl, Patrick Rapin, Reuben Thomas
-- completion from https://github.com/rrthomas/lua-rlcompleter
local exports = {}
exports.name = "console"
exports.version = "0.0.1"
exports.description = "Console plugin"
exports.license = "BSD-3-Clause"
exports.author = { name = "Carl" }

local console = exports
local history_file = "console_history"

local history_fullpath = nil

local reset_subscription, stop_subscription

function console.startplugin()
	local conth = emu.thread()
	local ln_started = false
	local started = false
	local stopped = false
	local ln = require("linenoise")
	local preload = false
	local matches = {}
	local lastindex = 0
	local consolebuf
	print("       /|  /|    /|     /|  /|    _______")
	print("      / | / |   / |    / | / |   /      /")
	print("     /  |/  |  /  |   /  |/  |  /  ____/ ")
	print("    /       | /   |  /       | /  /_     ")
	print("   /        |/    | /        |/  __/     ")
	print("  /  /|  /|    /| |/  /|  /|    /____    ")
	print(" /  / | / |   / |    / | / |        /    ")
	print("/ _/  |/  /  /  |___/  |/  /_______/     ")
	print("         /  /                            ")
	print("        / _/                             \n")
	print(emu.app_name() .. " " .. emu.app_version(), "\nCopyright (C) Nicola Salmoria and the MAME team\n");
	print(_VERSION, "\nCopyright (C) Lua.org, PUC-Rio\n");
	print([[
TAB completion:

  pressing TAB with = or ? at the end of the line will evaluate and print
      example:  emu.time()=  {1,2,3}=  os.date("*t")=   utf8.char(0x1f600)=

  pressing TAB with [ at the end of the line will do table completion
      example:  manager.machine.devices[
]])
	-- linenoise isn't thread safe but that means history can handled here
	-- that also means that bad things will happen if anything outside lua tries to use it
	-- especially the completion callback
	ln.historysetmaxlen(50)
	local scr = [[
		local ln = require('linenoise')
		ln.setcompletion(
			function(c, str)
				status = str
				yield()
				for candidate in status:gmatch('([^\001]+)') do
					ln.addcompletion(c, candidate)
				end
			end)
		local ret = ln.linenoise('$PROMPT')
		if ret == nil then
			return "\n"
		end
		return ret
	]]
	local keywords = {
		'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for',
		'function', 'if', 'in', 'local', 'nil', 'not', 'or', 'repeat',
		'return', 'then', 'true', 'until', 'while'
	}
	local cmdbuf = ""

	-- Main completion function. It evaluates the current sub-expression
	-- to determine its type. Currently supports tables fields, global
	-- variables and function prototype completion.
	local function contextual_list(expr, sep, str, word, strs)
		local function add(value)
			value = tostring(value)
			if value:match("^" .. word) then
				matches[#matches + 1] = value
			end
		end

		-- This function is called in a context where a keyword or a global
		-- variable can be inserted. Local variables cannot be listed!
		local function  add_globals()
			for _, k in ipairs(keywords) do
				-- add(k)
			end
			local sorttable = {}
			for k in pairs(_G) do
				if not k:match("^sol%.") then
					table.insert(sorttable, k)
				end
			end
			table.sort(sorttable)
			for k,v in pairs(sorttable) do add(v) end
			add("<Globals>  Esc to exit completions")
		end

		local function checkpairs(t) local p = pairs(t)  end
		local function checkfor(t) for k,v in pairs(t) do end end

		if expr and expr ~= "" then
			local v = load("local STRING = {'" .. table.concat(strs,"','") .. "'} return " .. expr)
			if v then
				err, v = pcall(v)
				if (not err) or (not v) then
					add_globals()
					return
				end
				local t = type(v)
				--print('SEP='..sep)
				if sep == '.' or sep == ':' then
					if t == 'table' then
						if pcall(checkfor, v) then
							local sorttable = {}
							for k, v in pairs(v) do
								if type(k) == 'string' and (sep ~= ':' or type(v) == "function") then
									table.insert(sorttable, k)
								end
							end
							table.sort(sorttable)
							for k,v in pairs(sorttable) do add(v) end
						end
					elseif t == 'userdata' then
						if pcall(checkfor,getmetatable(v)) then
							local sorttable = {}
							for k, v in pairs(getmetatable(v)) do
								if type(k) == 'string' and (sep ~= ':' or type(v) == "function") then
									table.insert(sorttable, k)
								end
							end
							table.sort(sorttable)
							for k,v in pairs(sorttable) do add(v) end
						end
					end
				elseif sep == '[' then
					if t == 'table' or t == 'userdata' then
						if pcall(checkpairs,v) then
							local sorttable = {}
							for k in pairs(v) do
								if type(k) == 'number' then
									table.insert(sorttable, k .. "]")
								end
								if type(k) == 'string' then
									table.insert(sorttable, "\"" .. k .. "\"]")
								end
							end
							table.sort(sorttable)
							for k,v in pairs(sorttable) do add(v) end
						end
						if word ~= "" then add_globals() end
					end
				end
			end
		end
		if #matches == 0 then
			add_globals()
		end
	end

	local function find_unmatch(str, openpar, pair)
		local done = false
		if not str:match(openpar) then
			return str
		end
		local tmp = str:gsub(pair, "")
		if not tmp:match(openpar) then
			return str
		end
		repeat
			str = str:gsub(".-" .. openpar .. "(.*)", function (s)
				tmp = s:gsub(pair, "")
				if not tmp:match(openpar) then
					done = true
				end
				return s
			end)
		until done or str == ""
		return str
	end

	-- This complex function tries to simplify the input line, by removing
	-- literal strings, full table constructors and balanced groups of
	-- parentheses. Returns the sub-expression preceding the word, the
	-- separator item ( '.', ':', '[', '(' ) and the current string in case
	-- of an unfinished string literal.
	local function simplify_expression(expr, word)
		local strs = {}
		-- Replace annoying sequences \' and \" inside literal strings
		expr = expr:gsub("\\(['\"])", function (c)
				return string.format("\\%03d", string.byte(c))
			end)
		local curstring
		-- Remove (finished and unfinished) literal strings
		while true do
			local idx1, _, equals = expr:find("%[(=*)%[")
			local idx2, _, sign = expr:find("(['\"])")
			if idx1 == nil and idx2 == nil then
				break
			end
			local idx, startpat, endpat
			if (idx1 or math.huge) < (idx2 or math.huge) then
				idx, startpat, endpat = idx1, "%[" .. equals .. "%[", "%]" .. equals .. "%]"
			else
				idx, startpat, endpat = idx2, sign, sign
			end
			if expr:sub(idx):find("^" .. startpat .. ".-" .. endpat) then
				expr = expr:gsub(startpat .. "(.-)" .. endpat, function (str)
						strs[#strs + 1] = str
						return " STRING[" .. #strs .. "] "
					end)
			else
				expr = expr:gsub(startpat .. "(.*)", function (str)
						curstring = str
						return "(CURSTRING "
					end)
			end
		end
		-- crop string at unmatched open paran
		expr = find_unmatch(expr, "%(", "%b()")
--      expr = find_unmatch(expr, "%[", "%b[]")
		--expr = expr:gsub("%b()"," PAREN ") -- Remove groups of parentheses
		expr = expr:gsub("%b{}"," TABLE ") -- Remove table constructors
		-- Avoid two consecutive words without operator
		expr = expr:gsub("(%w)%s+(%w)","%1|%2")
		-- expr = expr:gsub("%s+", "") -- Remove now useless spaces
		-- This main regular expression looks for table indexes and function calls.
		return curstring, strs, expr:match("([%.:%w%(%)%[%]_]-)([%:%.%[%(])" .. word .. "$")
	end

	local function number_then_string_sort(a, b)
		if type(a) == type(b) then
			return a < b -- Use default comparison if types match
		elseif type(a) == "number" then
			return true -- Numbers come before strings
		else
			return false -- Strings come after numbers
		end
	end

	function printt(t)
		print("TYPE = "..tostring(type(t)))
		-- if type(t)=="number" or type(t)=="boolean" or type(t)=="string" or type(t)==nil then
		print(tostring(t))
		if type(t)=="string" then print('"' .. tostring(t) .. '"') end
		if type(t)=="number" then
			if math.type(t)=="float" then
				print(math.type(t) .. "=" .. tostring(t))
			else
				print(math.type(t) .. "=" .. tostring(t) .. "   hex=" .. string.format("0x%x",t))
			end
		end
		if type(t)=="userdata" or type(t)=="table" then
			local sorttable = {}
			for i,j in pairs(t) do table.insert(sorttable,i) end
			table.sort(sorttable, number_then_string_sort)
			print("{")
			for i,j in pairs(sorttable) do
				if type(j)=="string" then io.write('["' .. j .. '"]')
				elseif type(j)=="number" then io.write('[' .. j .. ']')
				end
				print(" =  "..tostring(t[j])..",")
				--print(j,t[j])
			end
			print("}")
		end
	end


	local function print_line_eval(line)
		print()
		if not line:match("^%s*emu%s*[%?%=]%s*$") then
			print("Evaluating print( " .. line:sub(1,-2) .. " )")
			local func, err = load("printt(" .. line:sub(1,-2) .. ")")
			local status, result
			if func then
				status, result = pcall(func)
				if not status then print("Run error= "..tostring(result))
				else
	--              print(tostring(result))
				end
				return line
			else
				print("Cannot execute "..line:sub(1,-2))
				print("Load error="..tostring(err))
				return line
			end
		else
			local sorttable = {}
			for k in pairs(emu) do
				table.insert(sorttable, k)
			end
			table.sort(sorttable)
			for k, i in pairs(sorttable) do
				if i~="expression_error" and i~= "attotime" then print(i, emu[i]) else print(i, "<<<segfault warning>>>") end
			end
			--print("emu= will cause a crash")
		end
	end

	local function print_columns(printtable)
		local ncols = 4
		local colwidth = 30
		local numitems = #printtable
		local numlines = (numitems // ncols) + 1
		for l = 1, numlines do
			for i = l, l + numlines * ncols, numlines do
				if printtable[i] then
					io.write(tostring(printtable[i]) ..
						string.rep(" ", (colwidth - 2) - string.len(tostring(printtable[i]))) .. "  ")
				end
			end
			print()
		end
	end

	local function strnil(s)
		if type(s)=="table" then
			local ret = ""
			for i,j in pairs(s) do
				ret=ret..strnil(j)
			end
			return ret
		end
		if not s then return "nil" else return s end
	end

	local function get_completions(line)
		if line:match("[%?%=]$") then
			print_line_eval(line)
			return line
		end
		matches = {}
		local debug_completions = nil
		local start, word = line:match("^(.*[ \t\n\"\\'><=;:%+%-%*/%%^~#{}%(%)%[%].,])(.-)$")
		if debug_completions then print ("line="..strnil(line)) print ("start="..strnil(start).."  word="..strnil(word)) end
		if not start then
			start = ""
			word = word or line
		else
			word = word or ""
		end

		local str, strs, expr, sep = simplify_expression(line, word)
		if debug_completions then print ("expr="..strnil(expr).." sep="..strnil(sep).." word="..strnil(word).." strs="..strnil(strs)) end
		contextual_list(expr, sep, str, word, strs)
		if #matches == 0 then
			return line
		elseif #matches == 1 then
			return start .. matches[1]
		end
		print("")
		local result = { }
		local printtable = {}
		for k, v in pairs(matches) do
			table.insert(printtable, v)
			table.insert(result, start .. v)
		end
		print_columns(printtable)
		return table.concat(result, '\001')
	end

	reset_subscription = emu.add_machine_reset_notifier(function ()
		if not consolebuf and manager.machine.debugger then
			consolebuf = manager.machine.debugger.consolelog
			lastindex = 0
		end
	end)

	stop_subscription = emu.add_machine_stop_notifier(function ()
		consolebuf = nil
	end)

	emu.register_periodic(function ()
		if stopped then
			return
		end
		if (not started) then
			-- options are not available in startplugin, so we load the history here
			local homepath = manager.options.entries.homepath:value():match("([^;]+)")
			history_fullpath = homepath .. '/' .. history_file
			ln.loadhistory(history_fullpath)
			started = true
		end
		local prompt = "\x1b[1;36m[MAME]\x1b[0m> "
		if consolebuf and (#consolebuf > lastindex) then
			local last = #consolebuf
			print("\n")
			while lastindex < last do
				lastindex = lastindex + 1
				print(consolebuf[lastindex])
			end
			-- ln.refresh() FIXME: how to replicate this now that the API has been removed?
		end
		if conth.yield then
			conth:continue(get_completions(conth.result))
			return
		elseif conth.busy then
			return
		elseif ln_started then
			local cmd = conth.result
			if cmd == "\n" then
				stopped = true
				return
			elseif cmd == "" then
				if cmdbuf ~= "" then
					print("Incomplete command")
					cmdbuf = ""
				end
			else
				cmdbuf = cmdbuf .. "\n" .. cmd
				ln.historyadd(cmd)
				local func, err = load(cmdbuf)
				if not func then
					if err:match("<eof>") then
						prompt = "\x1b[1;36m[MAME]\x1b[0m>> "
					else
						print("error: ", err)
						cmdbuf = ""
					end
				else
					cmdbuf = ""
					stopped = true
					local status
					status, err = pcall(func)
					if not status then
						print("error: ", err)
					end
					stopped = false
				end
			end
		end
		conth:start(scr:gsub("$PROMPT", prompt))
		ln_started = true
	end)
end

setmetatable(console, {
			 __gc = function ()
				 if history_fullpath then
					 local ln = require("linenoise")
					 ln.savehistory(history_fullpath)
				 end
end})

return exports
