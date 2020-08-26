project = Project()

target = project:CreateBinary("test_httpkit"):AddDependencies(
    project:CreateDependency()
        :AddSourceFiles("*.c")
        :AddFlags({"-Wall", "-Werror", "-Wextra"})
        :AddStaticLibraries("..", "httpkit_static"))

return project
