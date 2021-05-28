#ifndef WEBSERVER_CLIENT_HPP
#define WEBSERVER_CLIENT_HPP

//custom includes
#include "header_handler.hpp"

class client {
public:
    friend class webserver;

private:
    //File descriptors
    int                 _clientFD;
    int                 _fileFD;
    int 	            _cgi_inputFD;
    bool                _authorization_status;

    //Handler
    header_handler      _handler;

    //Index
    int                 _index;

	//Time out check
	bool				_time_out_check;

public:
    client();
    client(int newFD, int client_amount);
    ~client();

    //GET functions
    int         get_clientFD();
    int         get_fileFD();
    int         get_cgi_inputFD();
    bool        get_authorization_status();
	bool		get_time_out_check();

    //SET functions
    void        set_clientFD(int fd);
    void        set_fileFD(int fd);
    void        set_cgi_inputFD(int fd);
    void        set_authorization_status(bool status);
	void		set_time_out_check(bool time_out_check);

};


#endif //WEBSERVER_CLIENT_HPP
