workspace "PyParser3"
	architecture "x86_64"
	startproject "PyParser3"
	configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
-- IncludeDir["freetype"] = "vendor/freetype-2.10.4/include"



-- group "Dependencies"
--     include "Aleph-Core/vendor/GLFW"
-- 	include "Aleph-Core/vendor/Glad"
-- 	include "Aleph-Core/vendor/Acrylic"
-- group ""

include "PyParser3"
-- include "PhysicsSim"