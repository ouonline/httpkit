project = CreateProject()

target = project:CreateLibrary("httpkit", STATIC | SHARED)
target:AddSourceFiles("*.c")
target:AddLibrary("../utils", "utils", STATIC)

return project
