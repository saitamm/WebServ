#include "../hpp/Response.hpp"
void handleDelete(Response &resp)
{
    struct stat path;
    string file = resp.getRequest().getUri();
    cout << "file is  = " << file << endl;
    if (file[0] == '/')
        file.erase(0, 1);
    cout << "file is after   = " << file << endl;
    if (stat(file.c_str(), &path) == -1)
    {
        cerr << "error with stat\n";
    }
    if (S_ISREG(path.st_mode))
    {
        if (unlink(file.c_str()) == -1)
        {
            resp.setStatus("403");
            return;
        }
        else
            cout << "file deleted\n";
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