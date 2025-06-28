#include "../hpp/Response.hpp"
void handleDelete(Response &resp)
{
    struct stat path;
    string file = resp.getRequest().getUri();
    if (file[0] == '/')
        file.erase(0, 1);
    if (stat(file.c_str(), &path) == -1)
    {
        cerr << "error with stat\n";
        resp.setStatus("403");
        std::string body = "<!DOCTYPE html>"
                   "<html>"
                   "<head>"
                   "<title>"+resp.getStatus()+resp.getValue(resp.getStatus()) +"</title>"
                   "</head>"
                   "<body style=\"margin: 0; height: 100vh; display: flex; justify-content: center; align-items: center; background-color: white;\">"
                   "<h1 style=\"color: red; font-size: 3em;\">403 Forbidden</h1>"
                   "</body>"
                   "</html>";

        resp.setBody(body);
        return;
    }
    if (S_ISREG(path.st_mode))
    {
        if (unlink(file.c_str()) == -1)
        {
            resp.setStatus("403");
            std::string body = "<!DOCTYPE html>"
                   "<html>"
                   "<head>"
                   "<title>"+resp.getStatus()+resp.getValue(resp.getStatus()) +"</title>"
                   "</head>"
                   "<body style=\"margin: 0; height: 100vh; display: flex; justify-content: center; align-items: center; background-color: white;\">"
                   "<h1 style=\"color: green; font-size: 3em;\">"+resp.getStatus()+" "+resp.getValue(resp.getStatus()) +"</h1>"
                   "</body>"
                   "</html>";
            return;
        }
        else
        {
            resp.setStatus("200");
            std::string body = "<!DOCTYPE html>"
                   "<html>"
                   "<head>"
                   "<title>"+resp.getStatus()+resp.getValue(resp.getStatus()) +"</title>"
                   "</head>"
                   "<body style=\"margin: 0; height: 100vh; display: flex; justify-content: center; align-items: center; background-color: white;\">"
                   "<h1 style=\"color: green; font-size: 3em;\">"+resp.getStatus()+resp.getValue(resp.getStatus()) +"</h1>"
                   "</body>"
                   "</html>";
            resp.setBody(body);
            cout << "file deleted\n";
        }
    }
    else if (S_ISDIR(path.st_mode))
    {
        cout << "is directory " << endl;
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