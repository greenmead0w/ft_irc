# ft_irc
ft_irc project for 42 Málaga.

*This project has been created as part of the 42 curriculum by [mzuloaga](https://profile-v3.intra.42.fr/users/mzuloaga) & [jotrujil](https://profile-v3.intra.42.fr/users/jotrujil)*

## Description 

This project consists of developing a fully functional IRC (Internet Relay Chat) server using C++ 98. The main goal is to build a solid base for real-time messaging, allowing multiple clients to connect simultaneously via the TCP/IP protocol without hanging.

The server handles user authentication, channel creation, and the exchange of private messages or group discussions. It also implements user roles and operator privileges for channel moderation, such as kicking or inviting users.

Instructions To compile and run the server, follow these steps according to the project requirements:

1.  Compilation: Use the provided Makefile to generate the ircserv executable. It must include the mandatory rules: all, clean, fclean, and re. Run in your terminal: make.

2.  Execution: The program requires a port and a network password to operate: ./ircserv <port> <password>.

3.  Connection: You can use any reference IRC client (like HexChat or Irssi) or even nc to conect to the server using the local IP and the chosen port.

## Resources 

The following sources were consulted during the development of this server:
-   All error & more codes are based on: [https://www.alien.net.au/irc/irc2numerics.html](https://www.alien.net.au/irc/irc2numerics.html)

-   RFC 1459 & 2812: Official IRC protocol documentation to understand message structures and commands.

-   We have been inspired by reading [this Medium article]([url](https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9)).

-   C++ 98 Tutorials: Various articles on memory management and STL containers availiable on the web.

## AI Usage Disclosure

In compliance with project rules, the use of AI tools is detailed below:

-   Tasks: AI was used to generate ideas for the command parser structure and to design "stress" test cases, such as handling partial commands and unextpected disconnections.

-   Parts of the project: Assistance focused mainly on the parameter validation logic for the MODE command and optimizing input/output buffers to ensure non-blocking behavior. All generated content was reviewed, tested, and fully understood to ensure server stability.
-   README: Yes, we also used AI to help us generate this document.

## Features

The server supports the following mandatory functionalities:

-   Authentication: PASS, NICK, and USER commands.

-   Channels: Joining channels and group messaging.

-   Privacy: Private messages (PRIVMSG) and notifications (NOTICE).

-   Operators: Privilege management with KICK, INVITE, TOPIC, and MODE (+i, +t, +k, +o, +l) commands.
