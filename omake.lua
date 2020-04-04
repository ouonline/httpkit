project = CreateProject()

target = project:CreateLibrary("httpkit")
target:AddSourceFile("*.c")
target:AddStaticLibrary("../utils", "utils")

return project
