#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP
#include <exception>
#include <string>
#include <iostream>
using namespace std;    


class BadRequestException : public exception
{
public:
    const char *what() const throw()
    {
        return ("400");
    }
};


class ConnectionFailedException : public exception
{
public:
    const char *what() const throw()
    {
        return ("-1");
    }
};

class NotFoundException : public exception
{
public:
    const char *what() const throw()
    {
        return ("404");
    }
};
class LargeHeaderException : public exception
{
public:
    const char *what() const throw()
    {
        return ("431");
    }
};

class NotImplementedException : public exception
{
public:
    const char *what() const throw()
    {
        return ("501");
    }
};

class ConflictException : public exception
{
public:
    const char *what() const throw()
    {
        return ("409");
    }
};
class SocketErrorException : public exception
{
public:
    const char *what() const throw()
    {
        return ("Bad Socket\n");
    }
};

class ServerErrorException : public exception
{
public:
    const char *what() const throw()
    {
        return ("500");
    }
};

class ForbiddenException : public exception
{
public:
    const char *what() const throw()
    {
        return ("403");
    }
};



class ContentLengthException : public exception
{
public:
    const char *what() const throw()
    {
        return ("411");
    }
};

class RedirectionException : public exception
{
public:
    const char *what() const throw()
    {
        return ("0");
    }
};
    
#endif