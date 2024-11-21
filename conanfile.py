import os
from conan import ConanFile
from conan.tools.files import copy


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
					'16': '2019',
					'17': '2022'}
		premake_command = "premake5 vs%s" % _visuals.get(str(self.settings.compiler.version), "UnknownVersion %s" % str(self.settings.compiler.version))
		self.run(premake_command)
	else:
		self.run("premake5 gmake")


class RpcConan(ConanFile):
	name = "rpc"
	version = "master"
	license = "MIT"
	author = "<Put your name here> <And your email here>"
	url = "https://github.com/Enhex/RPC"
	description = "Basic RPC over TCP."
	options = {
		"shared": [True, False],
		"system_openssl": [True, False],
		"openssl1": [True, False],
	}
	default_options = {
		"shared": False,
		"system_openssl": False,
		"openssl1": False,
	}
	exports_sources = "src/*"
	# headers only:
	# settings = "os", "compiler", "build_type", "arch"
	#generators = "PremakeDeps"
	no_copy_source = True

	def requirements(self):
		self.requires("high_level_asio/master", transitive_headers=True)
		if not self.options.system_openssl:
			if self.options.openssl1:
				self.requires("openssl/1.1.1w")
			else:
				self.requires("openssl/3.3.2")

	def package(self):
		copy(self, "*.h", os.path.join(self.source_folder, "src"), os.path.join(self.package_folder, "include"))

	def package_info(self):
		if self.options.system_openssl:
			self.cpp_info.system_libs = ["ssl", "crypto"]
		# For header-only packages, libdirs and bindirs are not used
		# so it's necessary to set those as empty.
		self.cpp_info.bindirs = []
		self.cpp_info.libdirs = []

	def package_id(self):
		self.info.clear()
