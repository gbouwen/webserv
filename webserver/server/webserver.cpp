/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserver.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: roybakker <roybakker@student.codam.nl>       +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/03/30 16:30:47 by roybakker     #+#    #+#                 */
/*   Updated: 2021/03/30 16:30:47 by roybakker     ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../gnl/get_next_line.hpp"
#include "webserver.hpp"
#include "server.hpp"

#include <iostream>
void webserver::print_struct() {
    std::cout << "Content of struct\n";

    for (std::vector<server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
        server current = *it;

        std::cout << "------------- BEGIN SERVER BLOCK -------------\n";
        std::cout << "Port = " << current.get_port() << std::endl;
        std::cout << "Host = " << current.get_host() << std::endl;
        std::cout << "File size = " << current.get_file_size() << std::endl;
        std::cout << "Server name = " << current.get_server_name() << std::endl;
        std::cout << "Error page = " << current.get_error_page() << std::endl;

        std::vector<location_context> locations = current.get_location();
        for (std::vector<location_context>::iterator it = locations.begin(); it != locations.end(); it++) {
            location_context location = *it;

            std::cout << "\n  ------------- location block -------------\n";
            std::cout << "  location = " << location.get_location() << std::endl;
            std::cout << "  Root = " << location.get_root() << std::endl;

            std::vector<std::string> methods = location.get_method();
            std::cout << "  Allowed methods = ";
            for (std::vector<std::string>::iterator it1 = methods.begin(); it1 != methods.end(); it1++) {
                std::string method = *it1;
                std::cout << method << " ";
            }
            std::cout << "\n";

            std::vector<std::string> indexen = location.get_index();
            std::cout << "  Index = ";
            for (std::vector<std::string>::iterator it2 = indexen.begin(); it2 != indexen.end(); it2++) {
                std::string index = *it2;
                std::cout << index << " ";
            }
            std::cout << "\n";
            std::cout << "  Autoindex = " << location.get_autoindex() << std::endl;
        }
        std::cout << "------------- END SERVER BLOCK -------------\n\n";
    }
}

webserver::webserver() : _servers(0), _highest_fd(-1), _request_fd(-1), _file_fd(-1) {}
webserver::~webserver(){}

void    webserver::load_configuration(char *config_file) {
    std::vector<std::string>    server_block;
    server                      server;
    char                        *line;
    int                         ret;
    int                         fd = open(config_file, O_RDONLY);

	if (fd == -1)
		throw std::invalid_argument("Error opening config file");
    while ((ret = get_next_line(fd, &line)) == 1) {
        server_block.push_back(line);
        free(line);
        if (check_server_block(server_block)) {
            server.create_new_server(server_block);
            _servers.push_back(server);
            server_block.clear();
        }
    }
	free(line);
    close(fd);
	if (ret == -1)
		throw std::runtime_error("Error while reading config file");
	if (!server_block.empty())
		throw std::invalid_argument("Error: missing '{' or '}' in config file");
    print_struct();                         //  PRINTING STRUCT
}

int     webserver::check_server_block(std::vector <std::string> server_block) {
    int         open_bracket = 0;
    int         closing_bracket = 0;
    std::string str;

    for (std::vector<std::string>::iterator it = server_block.begin(); it != server_block.end(); it++) {
        str = it->data();
        if ((int)str.find("{") != -1)
            open_bracket++;
        if ((int)str.find("}") != -1)
            closing_bracket++;
    }
    if (open_bracket == closing_bracket && open_bracket != 0) {
        return 1;
	}
    return 0;
}

void    webserver::establish_connection(){
        _servers[0].create_socket();
        _servers[0].bind_socket_address(_servers[0].get_port());
        _servers[0].create_connection(100); //CHECK LATER
}

void    webserver::run() {
    initialize_fd_sets();
    initialize_highest_fd();
    while (1)
    {
        _read_fds = _buffer_read_fds;
        _write_fds = _buffer_write_fds;

        add_sockets_to_read_fds();
        if (select(_highest_fd + 1, &_read_fds, &_write_fds, NULL, NULL) == -1)
			throw std::runtime_error("Select failed");
        else {
            printf("XX  ");
            accept_request();
            handle_request();
            read_requested_file();
            create_response();
        }
    }
}

void    webserver::initialize_fd_sets() {
    FD_ZERO(&_read_fds);
    FD_ZERO(&_write_fds);
    FD_ZERO(&_buffer_read_fds);
    FD_ZERO(&_buffer_write_fds);
}

void    webserver::initialize_highest_fd() {
    _highest_fd = _servers[0].get_tcp_socket();
}

int		webserver::highest_fd(int fd_one, int fd_two) {
    if (fd_one > fd_two)
        return (fd_one);
    return (fd_two);
}

void    webserver::add_sockets_to_read_fds() {
    FD_SET(_servers[0].get_tcp_socket(), &_read_fds);
}

void    webserver::accept_request() {
    if (FD_ISSET(_servers[0].get_tcp_socket(), &_read_fds))
    {
        _servers[0]._io_fd = accept(_servers[0].get_tcp_socket(), (struct sockaddr *)&_servers[0]._addr, (socklen_t *)&_servers[0]._addr_len);
        fcntl(_servers[0]._io_fd, F_SETFL, O_NONBLOCK);
        FD_SET(_servers[0]._io_fd, &_buffer_read_fds);
        _highest_fd = highest_fd(_highest_fd, _servers[0]._io_fd);
    }
}

// create get_location()
// create get_index()

std::string	get_filename(std::string file)
{
	std::string	filename;
	size_t		start;
	size_t		len;

	start = file.find(' ') + 1;
	len = start;
	while (file[len] != ' ' && file[len] != '\0')
		len++;
	filename = file.substr(start, len - start);
	return (filename);
}

std::string	get_location_from_request(std::string file)
{
	std::string	location;
	std::string	filename;

	location.append("/Users/roybakker/Desktop/webserv/html_css_testfiles/test_one.html"); // should be one of location blocks
	filename = get_filename(file);
	printf("filename = %s\n", filename.c_str());
	if (filename.compare("/") == 0 || filename.compare("/favicon.ico") == 0)
		location.append("/test_one.html"); // get_index_file();
	else
		location.append(filename);
	printf("location = %s\n", location.c_str());
	return (location);
}

void    webserver::handle_request() {
    std::string     request_headers;
    request_handler handler;
	std::string	    location;

    if (FD_ISSET(_servers[0]._io_fd, &_read_fds))
    {
        request_headers = handler.read_request(_servers[0]._io_fd);
        int ret = _servers[0].update_request_buffer(_servers[0]._io_fd, request_headers);
        if (ret == valid_) {
            FD_CLR(_request_fd, &_buffer_read_fds);
//            handler.parse_request(_servers[0]._request_buffer); //complete request needs parsing
            location = get_location_from_request(_servers[0]._request.get_file());
            _file_fd = _servers[0]._request.open_requested_file(location);
            if (_file_fd == -1)
            {
                std::cout << "file not found" << std::endl;
                // get an error page / favicon / something else
            }
            _highest_fd = highest_fd(_highest_fd, _file_fd);
            FD_SET(_file_fd, &_buffer_read_fds);
        }

//        _servers[0]._request.read_file(_request_fd);
//        FD_CLR(_servers[0]._io_fd, &_buffer_read_fds);
//        //some parsing functions needed to process request
//		location = get_location_from_request(_servers[0]._request.get_file());
//        _file_fd = _servers[0]._request.open_requested_file(location);
//		if (_file_fd == -1)
//		{
//			std::cout << "file not found" << std::endl;
//			// get an error page / favicon / something else
//		}
//		_highest_fd = highest_fd(_highest_fd, _file_fd);
//		FD_SET(_file_fd, &_buffer_read_fds);
    }
}

void    webserver::read_requested_file() {
    if (FD_ISSET(_file_fd, &_read_fds))
    {
        _servers[0]._response.read_file(_file_fd);
        FD_CLR(_file_fd, &_buffer_read_fds);
        FD_SET(_servers[0]._io_fd, &_buffer_write_fds);
    }
}

void    webserver::create_response() {
    if (FD_ISSET(_servers[0]._io_fd, &_write_fds))
    {
        _servers[0]._response.create_response_file(_servers[0]._io_fd, _servers[0]._response.get_file());
        FD_CLR(_servers[0]._io_fd, &_buffer_write_fds);
        close(_request_fd);
        _servers[0]._io_fd = -1;
		_servers[0]._request.clear_file();
		_servers[0]._response.clear_file();
    }
}
