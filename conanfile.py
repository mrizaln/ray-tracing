from conan import ConanFile

class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/10.1.1")
        self.requires("stb/cci.20230920")
        self.requires("concurrencpp/0.1.7")
        self.requires("boost-ext-ut/2.0.1")
