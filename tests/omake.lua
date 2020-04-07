project = CreateProject()

target = project:CreateBinary("test_httpkit")
target:AddSourceFiles("*.c")
target:AddLibrary("..", "httpkit", STATIC)

return project
