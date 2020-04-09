project = CreateProject()

target = project:CreateBinary("test_httpkit")
target:AddSourceFiles("*.c")
target:AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
target:AddStaticLibrary("..", "httpkit_static")

return project
