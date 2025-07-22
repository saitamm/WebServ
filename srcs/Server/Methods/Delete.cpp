#include "../../../Includes/Response.hpp"

void deleteRecursively(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
        throw BadDirectoryException();

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        std::string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == -1)
        {
            std::cerr << "Failed to stat: " << fullPath << std::endl;
            throw BadDirectoryException();
        }

        if (S_ISDIR(st.st_mode))
        {
            deleteRecursively(fullPath);
        }
        else
        {
            if (remove(fullPath.c_str()) != 0)
                throw BadDirectoryException();
        }
    }
    closedir(dir);
    if (rmdir(path.c_str()) != 0)
        throw BadDirectoryException();
}
void handleDelete(Response &resp)
{
    struct stat path;
    string file = resp.getRequest()->getConfigFile().getRoot() + resp.getRequest()->getUri();
    if (stat(file.c_str(), &path) == -1)
    {
        setCodeStatus(resp, 404);
        return;
    }
    // file

    if (S_ISREG(path.st_mode))
    {
        if (remove(file.c_str()) == -1)
        {
            setCodeStatus(resp, 403);
            return;
        }
        else
        {
            setCodeStatus(resp, 200);
        }
    }
    // directory
    else if (S_ISDIR(path.st_mode))
    {

        if (file[file.size() - 1] == '/')
        {
            if (access(file.c_str(), W_OK) == -1)
            {
                setCodeStatus(resp, 403);
                return;
            }
            try
            {
                deleteRecursively(file);
                setCodeStatus(resp, 204);
            }
            catch (const std::exception &e)
            {
                setCodeStatus(resp, 403);
            }

            return;
        }
        else
        {
            setCodeStatus(resp, 409);
            return;
        }
    }
    else
    {
        setCodeStatus(resp, 404);
    }
}