--[[
	useage example:
		premake4 gmake && make config=release server

	only designed to work with Linux for now
	
	TODO:
		get shit working
		copy data files to proper locations?
		optional: get non-linux shit working
]]


local paths = {
	libraries = "../../src/libraries",
	libparsec = "../../src/libparsec",
	parsec = "../../src/parsec",
	parsecserver = "../../src/parsec_server",
    parsecroot = "../../../../parsec_root", -- adnauseum("dotdotslash")
}


solution "ParsecSolution"
	language "C++"
	platforms { "x32" }
	includedirs { "../../src/libraries" }
	
	configurations { "Debug", "Release" }
	
	configuration "Debug"
		defines { "DEBUG" }
		flags "Symbols"
			
	configuration "Release"
		flags "Optimize"
	
	configuration "linux"
		includedirs { "/usr/include", "/usr/local/include", "/opt/include" }
	

project "client"
	kind "WindowedApp"
	location "build/client"
	targetdir "build/client"
	targetname "parsec"
	
	defines { "PARSEC_CLIENT" }
	
	includedirs {
		paths.libparsec.."/include",
		paths.parsec.."/include",
	}
	
	files {
		paths.libraries.."/**",
		paths.libparsec.."/**",
		paths.parsec.."/**",
	}
    configuration "not windows"
        -- These are apparently windows thingies
        excludes { paths.parsec.."/**.rc" }
	
	configuration "linux"
		links { "SDL", "SDL_mixer", "GL" }
        postbuildcommands 
        {
            "cp parsec "..paths.parsecroot.."/client/",
        }

project "server"
	kind "ConsoleApp"
	location "build/server"
	targetdir "build/server"
    targetname "parsec_server"
	
	defines { "PARSEC_SERVER" }
	
	-- TODO lots of crap probably
	-- needs to link with 
	
	includedirs {
		paths.libparsec.."/include",
		paths.parsecserver.."/include",
	}
	
	files {
		paths.libparsec.."/**",
		paths.parsecserver.."/**",
	}

    configuration "linux"
        links { "ncurses" }
        postbuildcommands 
        {
            "cp parsec_server "..paths.parsecroot.."/server/",
        }
	




