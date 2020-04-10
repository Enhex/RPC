from conans import ConanFile, CMake


# automatically choose Premake generator
def run_premake(self):
	if "Visual Studio" in self.settings.compiler:
		_visuals = {'8': '2005',
					'9': '2008',
					'10': '2010',
					'11': '2012',
					'12': '2013',
					'14': '2015',
					'15': '2017',
					'16': '2019'}
		premake_command = "premake5 vs%s" % _visuals.get(str(self.settings.compiler.version), "UnknownVersion %s" % str(self.settings.compiler.version))
		self.run(premake_command)
	else:
		self.run("premake5 gmake")

		
class RpcConan(ConanFile):
	name = "RPC"
	version = "master"
	license = "MIT"
	author = "<Put your name here> <And your email here>"
	url = "https://github.com/Enhex/RPC"
	description = "Basic RPC over TCP."
	topics = ("<Put some tag here>", "<here>", "<and here>")
	settings = "os", "compiler", "build_type", "arch"
	options = {"shared": [True, False]}
	default_options = "shared=False"
	generators = "premake"
	exports = "premake5.lua"
	exports_sources = "src/*"
	
	requires = (
		"high_level_asio/master@enhex/stable",
		"openssl/1.1.1f"
	)

	# def build(self):
	# 	run_premake(self)
	# 	self.run('build')

	def package(self):
		self.copy("*.h", dst="include", src="src")
		# self.copy("*.lib", dst="lib", keep_path=False)
		# self.copy("*.dll", dst="bin", keep_path=False)
		# self.copy("*.dylib*", dst="lib", keep_path=False)
		# self.copy("*.so", dst="lib", keep_path=False)
		# self.copy("*.a", dst="lib", keep_path=False)

	# def package_info(self):
	# 	self.cpp_info.libs = ["RPC"]
