#include "../hpp/Response.hpp"

void getContentType(string &real_path, Response &resp)
{
    string five = real_path.substr(real_path.size() - 5, real_path.size() - 1);
    string four = real_path.substr(real_path.size() - 4, real_path.size() - 1);
    string three = real_path.substr(real_path.size() - 3, real_path.size() - 1);
    if (".html" == five)
    {
        resp.setType("text/html");
        return ;
    }
    if ((".jpeg" == five || ".jpg" == four))
    {
        resp.setType("image/jpg");
        return ;
    }
    if (".png" == four)
    {
        resp.setType("image/png");
        return ;
    }
    if (".css" == four)
    {
        resp.setType("text/css");
        return ;
    }
    if (".js" == three)
    {
        resp.setType("application/javascript");
        return ;
    }
    resp.setType("application/octet-stream");
}

void handleGet(Response &resp)
{
    string real_path = resp.getRequest()->getUri();
    struct stat path;
    if (real_path[0] == '/')
        real_path.erase(0, 1);
    if (stat(real_path.c_str(), &path) == -1)
    {
        setCodeBodyStatus(resp, 404);
        return;
    }
    if (S_ISREG(path.st_mode))
    {
        ifstream file(real_path.c_str(), ios::binary);
        // if (!file.is_open())
        // {
        //     return buildErrorPage(403);
        // }
        std::string buffer((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        // ostringstream ss;
        // ss << file.rdbuf();
        resp.setStatus(200);
        resp.setBody(buffer);
        getContentType(real_path, resp);
    }
    //     else if (S_ISDIR(path.st_mode)) {
    //         // It's a directory
    //     }
    // } else {
    // File or directory doesn't exist
    // }
}