Import("env")

project_dir = env.subst("$PROJECT_DIR").replace("\\", "/")
env.Append(CPPDEFINES=[("PROJECT_DIR", env.StringifyMacro(project_dir))])
