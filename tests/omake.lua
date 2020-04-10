project = CreateProject()

dep = project:CreateDependency()
dep:AddSourceFiles("*.c")
dep:AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
dep:AddStaticLibrary("..", "httpkit_static")

target = project:CreateBinary("test_httpkit")
target:AddDependencies(dep)

return project
