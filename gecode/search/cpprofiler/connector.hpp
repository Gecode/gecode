/*
 *  Main authors:
 *     Maxim Shishmarev <maxim.shishmarev@monash.edu>
 *
 *  Contributing authors:
 *     Kevin Leo <kevin.leo@monash.edu>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Kevin Leo, 2017
 *     Christian Schulte, 2017
 *     Maxim Shishmarev, 2017
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#if defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif

#include <basetsd.h>
using ssize_t = SSIZE_T;

#else

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#endif

namespace Gecode { namespace CPProfiler {

    class Node {
        NodeUID node_;
        NodeUID parent_;
        int alt_;
        int kids_;

        NodeStatus status_;

        Option<std::string> label_;
        Option<std::string> nogood_;
        Option<std::string> info_;

    public:
        Node(NodeUID node, NodeUID parent, int alt, int kids, NodeStatus status);
        Node& set_node_thread_id(int tid);
        const Option<std::string>& label() const;
        Node& set_label(const std::string& label);
        const Option<std::string>& nogood() const;
        Node& set_nogood(const std::string& nogood);
        const Option<std::string>& info() const;
        Node& set_info(const std::string& info);
        int alt() const;
        int kids() const;
        NodeStatus status() const;
        NodeUID node_uid() const;
        NodeUID parent_uid() const;
        int node_id() const;
        int parent_id() const;
        int node_thread_id() const;
        int node_restart_id() const;
        int parent_thread_id() const;
        int parent_restart_id() const;
    };

    class Connector {
    private:
        MessageMarshalling marshalling;

        const unsigned int port;

        int socket_fd;
        bool _connected;
#ifdef _WIN32
        bool _winsock_started;
#endif

        static int send_all(int s, const char* buf, int* len);
        void send_over_socket();
        void send_raw_msg(const std::vector<char>& buf);

    public:
        Connector(unsigned int port);

        bool connected() const;

        /// connect to a socket via port specified in the construction (6565 by
        /// default)
        void connect();

        // sends START_SENDING message to the Profiler with a model name
        void start(const std::string& file_path = "", int execution_id = -1, bool has_restarts = false);
        void restart(int restart_id = -1);
        void done();

        /// disconnect from a socket
        void disconnect();

        void send_node(const Node& node);
        Node create_node(NodeUID node, NodeUID parent, int alt, int kids, NodeStatus status);
    };

    /*
     * Nodes
     */
    inline Node::Node(NodeUID node, NodeUID parent, int alt, int kids, NodeStatus status)
        : node_{node}, parent_{parent}, alt_(alt), kids_(kids), status_(status) {}

    inline Node&
    Node::set_node_thread_id(int tid) {
        node_.tid = tid;
        return *this;
    }

    inline const Option<std::string>&
    Node::label() const {
        return label_;
    }

    inline Node&
    Node::set_label(const std::string& label) {
        label_.set(label);
        return *this;
    }

    inline const Option<std::string>&
    Node::nogood() const {
        return nogood_;
    }

    inline Node&
    Node::set_nogood(const std::string& nogood) {
        nogood_.set(nogood);
        return *this;
    }

    inline const Option<std::string>&
    Node::info() const {
        return info_;
    }

    inline Node&
    Node::set_info(const std::string& info) {
        info_.set(info);
        return *this;
    }

    inline int
    Node::alt() const {
        return alt_;
    }
    inline int
    Node::kids() const {
        return kids_;
    }

    inline NodeStatus
    Node::status() const {
        return status_;
    }

    inline NodeUID
    Node::node_uid() const {
        return node_;
    }
    inline NodeUID
    Node::parent_uid() const {
        return parent_;
    }

    inline int
    Node::node_id() const {
        return node_.nid;
    }
    inline int
    Node::parent_id() const {
        return parent_.nid;
    }
    inline int
    Node::node_thread_id() const {
        return node_.tid;
    }
    inline int
    Node::node_restart_id() const {
        return node_.rid;
    }
    inline int
    Node::parent_thread_id() const {
        return parent_.tid;
    }
    inline int
    Node::parent_restart_id() const {
        return parent_.rid;
    }

    /*
     * Connector
     */
    inline Connector::Connector(unsigned int port)
        : port(port), socket_fd(-1), _connected(false)
#ifdef _WIN32
          ,
          _winsock_started(false)
#endif
    {
    }

    inline bool
    Connector::connected() const {
        return _connected;
    }

    /*
     * The following code is taken from:
     *   Beej's Guide to Network Programming
     *   http://beej.us/guide/bgnet/
     * with the following license:
     *
     * Beej's Guide to Network Programming is Copyright © 2015 Brian "Beej Jorgensen" Hall.
     * With specific exceptions for source code and translations, below, this work is licensed under the Creative Commons Attribution- Noncommercial- No Derivative Works 3.0 License. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/ or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
     * One specific exception to the "No Derivative Works" portion of the license is as follows: this guide may be freely translated into any language, provided the translation is accurate, and the guide is reprinted in its entirety. The same license restrictions apply to the translation as to the original guide. The translation may also include the name and contact information for the translator.
     * The C source code presented in this document is hereby granted to the public domain, and is completely free of any license restriction.
     * Educators are freely encouraged to recommend or supply copies of this guide to their students.
     * Contact beej@beej.us for more information.
     */
    inline int
    Connector::send_all(int s, const char* buf, int* len) {
        int total = 0;         // how many bytes we've sent
        int bytes_left = *len; // how many we have left to send
        ssize_t n = 0;         // An empty message succeeds without calling send.

        while (total < *len) {
            n = send(s, buf + total, static_cast<size_t>(bytes_left), 0);
            if (n == -1) {
                break;
            }
            total += static_cast<int>(n);
            bytes_left -= static_cast<int>(n);
        }

        *len = total; // return number actually sent here

        return (n == -1) ? -1 : 0; // return -1 on failure, 0 on success
    }

    inline void
    Connector::send_raw_msg(const std::vector<char>& buf) {
        uint32_t buf_size = static_cast<uint32_t>(buf.size());
        int buf_size_len = sizeof(uint32_t);
        send_all(socket_fd, reinterpret_cast<char*>(&buf_size), &buf_size_len);
        int buf_size_int = static_cast<int>(buf_size);
        send_all(socket_fd, reinterpret_cast<const char*>(buf.data()), &buf_size_int);
    }

    inline void
    Connector::send_over_socket() {
        if (!_connected)
            return;

        std::vector<char> buf = marshalling.serialize();

        send_raw_msg(buf);
    }

    inline void
    Connector::connect() {
        struct addrinfo hints, *server_info, *p;
        int rv;

#ifdef _WIN32
        // Initialise Winsock.
        WSADATA winsock_data;
        int startup_result = WSAStartup(MAKEWORD(2, 2), &winsock_data);
        if (startup_result != 0) {
            printf("WSAStartup failed with error: %d\n", startup_result);
            goto give_up;
        }
        _winsock_started = true;
#endif

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        rv = getaddrinfo("localhost", std::to_string(port).c_str(), &hints, &server_info);
        if (rv != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(rv) << "\n";
            goto give_up;
        }

        // loop through all the results and connect to the first we can
        for (p = server_info; p != nullptr; p = p->ai_next) {
            socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (socket_fd == -1) {
                // errno is set here, but we don't examine it.
                continue;
            }

            if (::connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
#ifdef _WIN32
                closesocket(socket_fd);
#else
                close(socket_fd);
#endif
                socket_fd = -1;
                // errno is set here, but we don't examine it.
                continue;
            }

            break;
        }

        // Connection failed; give up.
        if (p == nullptr) {
            freeaddrinfo(server_info);
            goto give_up;
        }

        freeaddrinfo(server_info); // all done with this structure

        _connected = true;

        return;
    give_up:
        _connected = false;
#ifdef _WIN32
        if (_winsock_started) {
            WSACleanup();
            _winsock_started = false;
        }
#endif
    }

    inline void
    Connector::start(const std::string& file_path, int execution_id, bool has_restarts) {
        /// extract fzn file name
        std::string base_name(file_path);
        {
            size_t pos = base_name.find_last_of('/');
            if (pos != std::string::npos) {
                base_name = base_name.substr(pos + 1, base_name.length() - pos - 1);
            }
        }

        std::string info;
        {
            std::stringstream ss;
            ss << "{";
            ss << "\"has_restarts\": " << (has_restarts ? "true" : "false") << "\n";
            ss << ",\"name\": " << "\"" << base_name << "\"" << "\n";
            if (execution_id != -1) {
                ss << ",\"execution_id\": " << execution_id;
            }
            ss << "}";
            info = ss.str();
        }

        marshalling.make_start(info);
        send_over_socket();
    }

    inline void
    Connector::restart(int restart_id) {

        std::string info;
        {
            std::stringstream ss;
            ss << "{";
            ss << "\"restart_id\": " << restart_id << "\n";
            ss << "}";
            info = ss.str();
        }

        marshalling.make_restart(info);
        send_over_socket();
    }

    inline void
    Connector::done() {
        marshalling.make_done();
        send_over_socket();
    }

    inline void
    Connector::disconnect() {
        if (socket_fd != -1) {
#ifdef _WIN32
            closesocket(socket_fd);
#else
            close(socket_fd);
#endif
            socket_fd = -1;
        }
        _connected = false;
#ifdef _WIN32
        if (_winsock_started) {
            WSACleanup();
            _winsock_started = false;
        }
#endif
    }

    inline void
    Connector::send_node(const Node& node) {
        if (!_connected)
            return;

        auto& msg = marshalling.make_node(node.node_uid(), node.parent_uid(), node.alt(), node.kids(), node.status());

        if (node.label().valid())
            msg.set_label(node.label().value());
        if (node.nogood().valid())
            msg.set_nogood(node.nogood().value());
        if (node.info().valid())
            msg.set_info(node.info().value());

        send_over_socket();
    }

    inline Node
    Connector::create_node(NodeUID node, NodeUID parent, int alt, int kids, NodeStatus status) {
        return Node(node, parent, alt, kids, status);
    }

}} // namespace Gecode::CPProfiler

// STATISTICS: search-trace
