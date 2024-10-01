
project "PyParser3"
    kind "ConsoleApp"
    architecture "x86_64"
    
    language "C++"
    targetdir "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{prj.name}"
    objdir "../bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{prj.name}"
    debugdir "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{prj.name}"

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs
	{
		"/src",	
        "./include/"
	}

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
	

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    
    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

