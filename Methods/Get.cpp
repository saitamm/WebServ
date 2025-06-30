#include "../hpp/Response.hpp"

string getContentType(string &real_path)
{
    string five = real_path.substr(real_path.size() - 5, real_path.size() -1);
    string four = real_path.substr(real_path.size() - 4, real_path.size() -1);
    string three = real_path.substr(real_path.size() - 3, real_path.size() -1);
    if (".html" == five)
        return "text/html";
    if ((".jpeg" == five || "jpg" == four))
        return "image/jpeg";
    if (".png" == four)
        return "image/png";
    if (".css" == four)
        return "text/css";
    if (".js" == three)
        return ("application/javascript");
    return "application/octet-stream";
}

void generateResponse(string &content, string &real_path)
{
    ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-type: " << getContentType(real_path) << "\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << content;
    string final_resp = response.str();
}

void handleGet(Response& resp)
{
    string real_path = resp.getRequest().getConfigFile().getRoot() + resp.getRequest().getUri();
    struct stat path;
    if (!stat(real_path.c_str(), &path)) {
        if (S_ISREG(path.st_mode))
        {
            ifstream file(real_path.c_str(), ios::binary);
            // if (!file.is_open())
            // {
            //     return buildErrorPage(403);
            // }
            ostringstream ss;
            ss << file.rdbuf();
            string content = ss.str();
            generateResponse(content, real_path);
        
        }
    //     else if (S_ISDIR(path.st_mode)) {
    //         // It's a directory
    //     }
    // } else {
        // File or directory doesn't exist
        // }

    }
}
