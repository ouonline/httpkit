project = CreateProject()

dep = project:CreateDependency()
dep:AddSourceFiles("*.c")
dep:AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
dep:AddStaticLibrary("../utils", "utils_static")

a = project:CreateStaticLibrary("httpkit_static")
a:AddDependencies(dep)

so = project:CreateSharedLibrary("httpkit_shared")
so:AddDependencies(dep)

return project
