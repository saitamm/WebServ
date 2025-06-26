#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
class Response
{
public:
    Response();
    ~Response();
    void MakeResponce(Request &req);

private:
    Request _req;
};
#endif