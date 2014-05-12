cuetable = {
	--LOD1
	[0] = function() run2("lod1.lua"); move(0,1500,0); look(-90,0); wf(0); speed(100); tess(1) end,
	[1] = function() wf(0) cmove(-150,0,0); end,
	[2] = function() run2("lod1.lua"); cmove(-150,0,0); wf(1) end,
	
	--LOD2
	
	[3] = function() run2("lod2.lua"); move(0,1500,0); look(-90,0); wf(0); speed(100); tess(1) end,
	[4] = function() wf(1) end,
	[5] = function() cmove(-150,0,0); end,

	--Lod2 closeup
	
	[6] = function() move(0,400,0); look(-50,45); wf(1); tess(1) cmove(-150,0,0) end,
	
	[7] = function() wf(0) tess(1) cmove(-150,0,0) end,
	
	[8] = function() tess(2) end,
	[9] = function() tess(4) end,
	[10] = function() tess(8) end,
	[11] = function() tess(16) end,
	[12] = function() tess(32) wf(0) end,
	[13] = function() tess(32) wf(1) end,
	[14] = function() run2("lod2.lua") tess(1) cmove(-150,0,0) look(-50,45) wf(1) end,
	
	--City tile intro
	
	[15] = function() run2("citytile.lua") move(0,300,0) look(0,45) wf(0) end,
	[16] = function() cmove(-500,0,0) wf(0) end,
	[17] = function() run2("citytile.lua") wf(1) end,
	--The Result

	[18] = function() run2("mountains.lua"); move(39700,500,15104); look(0,45); wf(0) tess(32) end,
	[19] = function() move(39700,500,15104); cmove(-500,0,0) end,
	[20] = function() smove(0,30000,15000,0.10) slook(0,45,1) end,
	[21] = function() smove(0,100000,15000,0.10) slook(-30,45,0.1) end,
	[22] = function() smove(-2000000,10000,15000,0.50) end,
	[23] = function() smove(-2000000,1000,15000,0.50) slook(-30,45,0.1) end,
	[24] = function() move(-2000000,40000,15000) cmove(-1000000,0,0) slook(0,45,1) end,
	[25] = function() run2("mountains.lua") cmove(0,0,0) slook(-30,45,1)  end,
	[26] = function() run2("cube.lua") move(0,2000,0) look(0,90) clook(0,5) end,
	[27] = function() run2("water.lua") move(0,1000,0) look(-20,-90) cmove(0,0,150) end,
}

if cue < 0 then cue = 0 end
if cue > 50 then cue = 50 end

print("cue = ");
print(cue);
cuetable[cue]();
