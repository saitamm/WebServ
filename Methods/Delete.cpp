#include "../hpp/Response.hpp"
void setErrorBodyStatus(Response &resp, int error)
{
    resp.setStatus(error);
    map<int, string> body = resp.getRequest().getConfigFile().getError_page();
    if (body[error][0] == '/')
        body[error].erase(0, 1);
    fstream file(body[error].c_str());
    cout << "error page = " << body[error] << endl;
    if (!file.is_open())
    {
        std::cerr << "❌ Failed to open file: " << body[error] << std::endl;
        return;
    }
    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    resp.setBody(buffer);
}
void handleDelete(Response &resp)
{
    struct stat path;
    string file = resp.getRequest().getUri();
    if (file[0] == '/')
        file.erase(0, 1);
    if (stat(file.c_str(), &path) == -1)
    {
        setErrorBodyStatus(resp, 403);
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
            cout << "i am to recurson\n";
            return;
        }
        else
        {
            setErrorBodyStatus(resp, 409);
            return;
        }
    }
    else
    {
        cout << "no one \n";
    }
    // string     file = resp.getRequest().getUri();
    // cout << "file is  = "<< file <<endl;
    // struct stat Buff;
    // cout << stat(file.c_str(), &Buff) << endl;
    // if (access(file.c_str(), F_OK))
    //     resp.setStatus("404");
}