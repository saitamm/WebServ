#include "../hpp/Response.hpp"
void setErrorBodyStatus(Response &resp, int error)
{
    resp.setStatus(error);
    map<int, string> body = resp.getRequest().getConfigFile().getError_page();
    if (body[error][0] == '/')
        body[error].erase(0, 1);
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
void setSuccessBodyStatus(Response &resp, int error)
{
    resp.setStatus(error);
    map<int, string> body = resp.getRequest().getConfigFile().getError_page();
    if (body[error][0] == '/')
        body[error].erase(0, 1);
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
    {
        std::cerr << "Failed to open directory: " << path << std::endl;
        return;
    }

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
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            deleteRecursively(fullPath);
        }
        else
        {
            if (unlink(fullPath.c_str()) != 0)
            {
                std::cerr << "Failed to delete file: " << fullPath << std::endl;
            }
        }
    }

    closedir(dir);

    // Delete the empty directory
    if (rmdir(path.c_str()) != 0)
    {
        std::cerr << "Failed to remove directory: " << path << std::endl;
    }
}
void handleDelete(Response &resp)
{
    struct stat path;
    string file = resp.getRequest().getUri();
    if (file[0] == '/')
        file.erase(0, 1);
    if (stat(file.c_str(), &path) == -1)
    {
        setErrorBodyStatus(resp, 404);
        return;
    }
    if (S_ISREG(path.st_mode))
    {
        if (unlink(file.c_str()) == -1)
        {
            setErrorBodyStatus(resp, 403);
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
            cout << "file deleted\n";
        }
    }
    else if (S_ISDIR(path.st_mode))
    {
        if (file[file.size() - 1] == '/')
        {
            if (access(file.c_str(), W_OK) == -1)
            {
                setErrorBodyStatus(resp, 403);
                return;
            }
            try
            {
                deleteRecursively(file);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
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
            cout << "file deleted\n";
            return;
        }
        else
        {
            cout << "=======\n"
                 << endl;
            setErrorBodyStatus(resp, 409);
            return;
        }
    }
    else
    {
        setErrorBodyStatus(resp, 404);
    }
    // string     file = resp.getRequest().getUri();
    // cout << "file is  = "<< file <<endl;
    // struct stat Buff;
    // cout << stat(file.c_str(), &Buff) << endl;
    // if (access(file.c_str(), F_OK))
    //     resp.setStatus("404");
}