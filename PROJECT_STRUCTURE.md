# WebServ - Improved Project Structure

## 📁 Recommended Directory Layout

```
WebServ/
├── 📁 src/                          # Source code
│   ├── 📁 core/                     # Core server functionality
│   │   ├── Server.cpp              # Main server class
│   │   ├── Server.hpp              
│   │   ├── ServerEngine.cpp        # Event loop and request handling
│   │   ├── ServerEngine.hpp        
│   │   └── Socket.cpp              # Socket management
│   │   └── Socket.hpp              
│   │
│   ├── 📁 http/                     # HTTP protocol handling
│   │   ├── 📁 request/             
│   │   │   ├── Request.cpp         # HTTP request parsing
│   │   │   ├── Request.hpp         
│   │   │   └── RequestParser.cpp   # Request parsing logic
│   │   │   └── RequestParser.hpp   
│   │   │
│   │   ├── 📁 response/            
│   │   │   ├── Response.cpp        # HTTP response generation
│   │   │   ├── Response.hpp        
│   │   │   ├── ResponseBuilder.cpp # Response building logic
│   │   │   └── ResponseBuilder.hpp 
│   │   │
│   │   └── 📁 methods/             # HTTP method handlers
│   │       ├── MethodHandler.cpp   # Base method handler
│   │       ├── MethodHandler.hpp   
│   │       ├── GetHandler.cpp      # GET method
│   │       ├── GetHandler.hpp      
│   │       ├── PostHandler.cpp     # POST method
│   │       ├── PostHandler.hpp     
│   │       ├── DeleteHandler.cpp   # DELETE method
│   │       └── DeleteHandler.hpp   
│   │
│   ├── 📁 config/                   # Configuration management
│   │   ├── ConfigFile.cpp          
│   │   ├── ConfigFile.hpp          
│   │   ├── ConfigParser.cpp        # Configuration parsing
│   │   ├── ConfigParser.hpp        
│   │   ├── Location.cpp            # Location block handling
│   │   └── Location.hpp            
│   │
│   ├── 📁 client/                   # Client connection management
│   │   ├── Client.cpp              
│   │   ├── Client.hpp              
│   │   ├── ClientManager.cpp       # Client lifecycle management
│   │   └── ClientManager.hpp       
│   │
│   ├── 📁 cgi/                      # CGI handling
│   │   ├── CgiHandler.cpp          
│   │   ├── CgiHandler.hpp          
│   │   ├── CgiExecutor.cpp         # CGI script execution
│   │   └── CgiExecutor.hpp         
│   │
│   ├── 📁 utils/                    # Utility functions
│   │   ├── Logger.cpp              # Logging functionality
│   │   ├── Logger.hpp              
│   │   ├── FileUtils.cpp           # File operations
│   │   ├── FileUtils.hpp           
│   │   ├── StringUtils.cpp         # String manipulation
│   │   ├── StringUtils.hpp         
│   │   ├── TimeUtils.cpp           # Time utilities
│   │   └── TimeUtils.hpp           
│   │
│   └── 📁 exceptions/               # Custom exceptions
│       ├── WebServException.cpp    # Base exception class
│       ├── WebServException.hpp    
│       ├── HttpException.cpp       # HTTP-specific exceptions
│       ├── HttpException.hpp       
│       ├── ConfigException.cpp     # Config-related exceptions
│       └── ConfigException.hpp     
│
├── 📁 include/                      # Header files (if separate from src)
│   └── webserv.h                   # Main header with common includes
│
├── 📁 config/                       # Configuration files
│   ├── default.conf                # Default server configuration
│   ├── example.conf                # Example configuration
│   └── 📁 error_pages/             # Custom error pages
│       ├── 400.html
│       ├── 403.html
│       ├── 404.html
│       ├── 405.html
│       ├── 500.html
│       └── 502.html
│
├── 📁 www/                          # Web root directory
│   ├── index.html                  # Default page
│   ├── 📁 static/                  # Static files
│   │   ├── 📁 css/
│   │   ├── 📁 js/
│   │   ├── 📁 images/
│   │   └── 📁 fonts/
│   │
│   ├── 📁 cgi-bin/                 # CGI scripts
│   │   ├── test.py
│   │   ├── form_handler.php
│   │   └── upload.cgi
│   │
│   └── 📁 uploads/                 # File upload directory
│
├── 📁 tests/                        # Test files
│   ├── 📁 unit/                    # Unit tests
│   │   ├── test_request.cpp
│   │   ├── test_response.cpp
│   │   ├── test_config.cpp
│   │   └── test_utils.cpp
│   │
│   ├── 📁 integration/             # Integration tests
│   │   ├── test_http_methods.cpp
│   │   ├── test_cgi.cpp
│   │   └── test_chunked.cpp
│   │
│   └── 📁 scripts/                 # Test scripts
│       ├── test_chunked.py
│       ├── stress_test.py
│       └── benchmark.sh
│
├── 📁 docs/                         # Documentation
│   ├── README.md                   # Main documentation
│   ├── API.md                      # API documentation
│   ├── CONFIGURATION.md            # Configuration guide
│   └── DEVELOPMENT.md              # Development guide
│
├── 📁 build/                        # Build artifacts (gitignored)
│   ├── 📁 obj/                     # Object files
│   └── 📁 bin/                     # Executables
│
├── Makefile                         # Build system
├── .gitignore                      # Git ignore file
├── AUTHORS                         # Project authors
└── LICENSE                         # License file
```

## 📝 **File Organization Benefits**

### 1. **Separation of Concerns**
- Each directory has a specific responsibility
- Easy to locate and modify specific functionality
- Reduced coupling between components

### 2. **Scalability** 
- Easy to add new HTTP methods
- Simple to extend configuration options
- Modular design for new features

### 3. **Maintainability**
- Clear naming conventions
- Logical file grouping
- Easier debugging and testing

### 4. **Professional Structure**
- Industry-standard layout
- Better collaboration support
- Clear development workflow
