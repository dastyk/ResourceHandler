# ResourceHandler
Part of the Editor project

A binary file system which concatenates files to one binary file.
A Multi-threaded Resource Handler.
To load a resource first create a resource handler then create a resource.

- [ ] As soon as the resource is created the resource handler will try to load the resource if there is enough free memory
- [x] Reference counting with no risk of dangling resources
- [x] Checking in a resource tells the resource handler that the resource is needed right now and will prioritize it.
- [x] The one creating the resource does not need to have a pointer to the resource handler, this is "under the hood"

# Dependencis
* CompileTimeGUID
* Profiler

# Build
If using Visual Studio (2017) just get each dependency so that they are located in the same folder then you should only need to press build.

If not using Visual Studio I wish you the best of luck.
