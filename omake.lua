project = Project()

dep = project:CreateDependency()
    :AddSourceFiles("*.c")
    :AddFlags({"-Wall", "-Werror", "-Wextra", "-fPIC"})
    :AddStaticLibraries("../utils", "utils_static")

project:CreateStaticLibrary("httpkit_static"):AddDependencies(dep)
project:CreateSharedLibrary("httpkit_shared"):AddDependencies(dep)

return project
