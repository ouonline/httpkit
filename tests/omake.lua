project = CreateProject()

target = project:CreateBinary("test_httpkit")
target:AddSourceFile("*.c")
target:AddStaticLibrary("..", "httpkit")

return project
