#include "../../../Includes/Response.hpp"
void setCodeBodyStatus(Response &resp, int error)
{
    resp.setStatus(error);
    map<int, string> body = resp.getRequest()->getConfigFile().getError_page();
    if (body[error][0] == '/')
    body[error].erase(0, 1);
    getContentType(body[error], resp);
    fstream file(body[error].c_str());
    if (!file.is_open())
    {
        std::cerr << "❌ Failed to open file: " << body[error] << std::endl;
        return;
    }
    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    resp.setBody(buffer);

}
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
    string file = resp.getRequest()->getUri();
    if (file[0] == '/')
        file.erase(0, 1);
    if (stat(file.c_str(), &path) == -1)
    {
        setCodeBodyStatus(resp, 404);
        return;
    }
    // file
    if (S_ISREG(path.st_mode))
    {
        if (remove(file.c_str()) == -1)
        {
            setCodeBodyStatus(resp, 403);
            return;
        }
        else
        {
            resp.setStatus(200);
            fstream file("errors/200.html");
            if (!file.is_open())
            {
                std::cerr << "❌ Failed to open file: " << std::endl;
                return;
            }
            std::string buffer((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            resp.setBody(buffer);
        }
    }
    // directory
    else if (S_ISDIR(path.st_mode))
    {
        if (file[file.size() - 1] == '/')
        {
            if (access(file.c_str(), W_OK) == -1)
            {
                setCodeBodyStatus(resp, 403);
                return;
            }
            try
            {
                deleteRecursively(file);
                setCodeBodyStatus(resp, 204);
            }
            catch (const std::exception &e)
            {
                setCodeBodyStatus(resp, 403);
            }

            return;
        }
        else
        {
            setCodeBodyStatus(resp, 409);
            return;
        }
    }
    else
    {
        setCodeBodyStatus(resp, 404);
    }
}