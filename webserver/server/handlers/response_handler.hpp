/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   response_handler.hpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: roybakker <roybakker@student.codam.nl>       +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/04/07 10:46:09 by roybakker     #+#    #+#                 */
/*   Updated: 2021/04/07 10:46:09 by roybakker     ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_RESPONSE_HANDLER_HPP
#define WEBSERV_RESPONSE_HANDLER_HPP

#include "handler.hpp"

class response_handler : public handler {
private:


public:
    response_handler();
    ~response_handler();

    void    create_response_file(int request_fd, std::string response_file);
};


#endif //WEBSERV_RESPONSE_HANDLER_HPP
