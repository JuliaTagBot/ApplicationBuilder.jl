if is_windows()
	include("installer.jl")
end
function build_app_bundle(script::String;
				resources = String[],
				libraries = String[],
				builddir = "builddir",
				appname = "nothing",
				create_installer = false)


	# Create build directory
	script = abspath(script)
	base_path = dirname(script)
	builddir = joinpath(base_path, builddir, appname)
	info("Building at path $builddir")
	mkpath(builddir)

	core_path = joinpath(builddir, "core")
	lib_path = joinpath(builddir, "lib")
	res_path = joinpath(builddir, "res")

	# Create resources and libraries dirctories
	mkpath(core_path)
	mkpath(lib_path)
	mkpath(res_path)

	delim = is_windows() ? '\\' : '/'

	info("Copying resources:")
	for res in resources
		print("Copying $res...")
		cp(res, joinpath(res_path, split(res, delim)[end]), remove_destination = true)
		println("... done.")
	end

	info("Copying libraries")
	for lib in libraries
		print("Copying $lib...")
		cp(lib, joinpath(lib_path, split(lib, delim)[end]), remove_destination = true)
		println("... done.")
	end

	build_executable(script, builddir = core_path)

    (create_installer && is_linux()) && throw(error("Cannot create installer on Linux"))

    if is_windows()
        create_installer && installer(builddir, name = appname)
    end

end