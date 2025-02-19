from conans import ConanFile

class SupercellWxConan(ConanFile):
    settings   = ("os", "compiler", "build_type", "arch")
    requires   = ("boost/1.85.0",
                  "cpr/1.10.5",
                  "fontconfig/2.15.0",
                  "freetype/2.13.2",
                  "geographiclib/2.3",
                  "geos/3.12.0",
                  "glew/2.2.0",
                  "glm/cci.20230113",
                  "gtest/1.14.0",
                  "libcurl/8.6.0",
                  "libxml2/2.12.6",
                  "openssl/3.2.1",
                  "re2/20231101",
                  "spdlog/1.14.1",
                  "sqlite3/3.46.0",
                  "vulkan-loader/1.3.243.0",
                  "zlib/1.3.1")
    generators = ("cmake",
                  "cmake_find_package",
                  "cmake_paths")
    default_options = {"geos:shared"      : True,
                       "libiconv:shared"  : True,
                       "openssl:no_module": True,
                       "openssl:shared"   : True}

    def requirements(self):
        if self.settings.os == "Linux":
            self.requires("onetbb/2021.12.0")

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib", dst="bin", src="lib")
        self.copy("license*", dst="licenses", src=".", folder=True, ignore_case=True)
