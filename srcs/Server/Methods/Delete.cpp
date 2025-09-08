#include "../../../Includes/Response.hpp"

void deleteRecursively(const string &path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
        throw ForbiddenException();

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == -1)
        {
            throw ForbiddenException();
        }
        if (S_ISDIR(st.st_mode))
        {
            deleteRecursively(fullPath);
        }
        else
        {
            if (remove(fullPath.c_str()) != 0)
                throw ForbiddenException();
        }
    }
    closedir(dir);
    if (rmdir(path.c_str()) != 0)
        throw ForbiddenException();
}
void handleDelete(Response &resp)
{
    struct stat path;
    string file = resp.getRequest()->getConfigFile().getRoot() + "/" + resp.getRequest()->getLocation().getUp_store() + resp.getRequest()->getUri();
    if (resp.getRequest()->getLocation().getUp_store().empty() || resp.getRequest()->getLocation().getUp_store() == "/")
        throw ForbiddenException();
    if (stat(file.c_str(), &path) == -1)
    {
        setCodeStatus(resp, 404);
        return;
    }
    // file
    if (S_ISREG(path.st_mode))
    {
        if (remove(file.c_str()) == -1)
            throw ForbiddenException();
        else
            setCodeStatus(resp, 200);
    }
    // directory
    else if (S_ISDIR(path.st_mode))
    {
        if (file[file.size() - 1] == '/')
        {
            if (file == resp.getRequest()->getConfigFile().getRoot() + "/" + resp.getRequest()->getLocation().getUp_store() + "/" || access(file.c_str(), W_OK) == -1)
                throw ForbiddenException();

            deleteRecursively(file);
            setCodeStatus(resp, 200);
            return;
        }
        else
            throw ConflictException();
    }
    else
        throw NotFoundException();
}