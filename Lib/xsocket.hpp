#pragma once

#if defined(_WIN32)
#include <WinSock2.h>
#pragma comment(lib, "wsock32.lib")

typedef SOCKET socket_handle_t;
typedef int socklen_t;
typedef int socket_error_t;

#define socket_close closesocket

#else

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int socket_handle_t;
typedef int socket_error_t;

#define socket_close ::close

#endif

namespace xsocket
{
	static const in_addr socket_addr_any = { INADDR_ANY };
	class socket_wait;

	inline void socket_init()
	{
	#if defined(_WIN32)
		WSADATA wsa;
		USHORT vers = 0x0202;
		WSAStartup(vers, &wsa);
	#endif
	}

	inline void socket_shutdown()
	{
	#if defined(_WIN32)
		WSACleanup();
	#endif
	}

	inline socket_error_t socket_error()
	{
#if defined(_WIN32)
		return WSAGetLastError();
#else
		return errno;
#endif
	}

	inline std::string socket_error_text(socket_error_t err)
	{
#if defined(_WIN32)
		std::string tmp;
		tmp.resize(64);
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			(DWORD)err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			tmp.data(),
			64,
			nullptr);
		return tmp;
#else
		return strerror(err);
#endif
	}

	class socket_base
	{
		friend class socket_wait;
	public:
		socket_base() = default;
		socket_base(const socket_base&) = default;
		socket_base(socket_base&&) = default;

		virtual ~socket_base()
		{

		}

	protected:
		socket_base(int af, int type, int protocol)
		{
			m_address_family = af;
			m_socket = socket(af, type, protocol);
		}

		socket_base(int af, int type, int protocol, uint16_t port)
			: socket_base(af, type, protocol, port, socket_addr_any)
		{
		}

		socket_base(int af, int type, int protocol, uint16_t port, in_addr address)
			: socket_base(af, type, protocol)
		{
			if (valid())
			{
				m_address.sin_family = af;
				m_address.sin_addr = address;
				m_address.sin_port = htons(port);
			}
		}

	public:
		inline operator bool() { return m_socket > 0; }
		inline bool valid() { return m_socket > 0; }

		int bind()
		{
			if (!m_socket)
				return -1;

			return ::bind(m_socket, reinterpret_cast<const sockaddr*>(&m_address), sizeof(sockaddr_in));
		}

		int bind(uint16_t port)
		{
			return bind(socket_addr_any, port);
		}

		int bind(in_addr address, uint16_t port)
		{
			m_address.sin_family = m_address_family;
			m_address.sin_addr = address;
			m_address.sin_port = htons(port);
			return bind();
		}

		void close()
		{
			if (m_socket)
				socket_close(m_socket);
		}

		void blocking(bool block)
		{
			if (!valid())
				return;

			unsigned long arg = (block) ? 0 : 1;

		#if defined(_WIN32)
			ioctlsocket(m_socket, FIONBIO, &arg);
		#else
			fcntl(m_socket, F_SETFL, O_NONBLOCK, arg);
		#endif
		}

		void buffer_size(int *send, int *recv)
		{
			if (!valid())
				return;

			if (send)
				setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)send, sizeof(int));
			if (recv)
				setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)recv, sizeof(int));
		}

	protected:
		int m_address_family;
		socket_handle_t m_socket;
		sockaddr_in m_address;
	};

	class socket_udp : public socket_base
	{
	public:
		socket_udp() :
			socket_base(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
		{ }

		socket_udp(uint16_t port) :
			socket_base(AF_INET, SOCK_DGRAM, IPPROTO_UDP, port)
		{ }

		socket_udp(uint16_t port, in_addr address) :
			socket_base(AF_INET, SOCK_DGRAM, IPPROTO_UDP, port, address)
		{ }

		int recvfrom(uint8_t *buffer, size_t length, sockaddr_in *addr_from)
		{
			socklen_t addr_len = sizeof(sockaddr_in);
			return ::recvfrom(m_socket, (char*)buffer, length, 0, (sockaddr*)addr_from, &addr_len);
		}

		int sendto(uint8_t *buffer, size_t length, sockaddr_in *addr_to)
		{
			static const socklen_t addr_len = sizeof(sockaddr_in);
			return ::sendto(m_socket, (char*)buffer, length, 0, (sockaddr*)addr_to, addr_len);
		}
	};

	class socket_wait
	{
	public:
		socket_wait() :
			m_fd({ 0 }), m_delay({ 1, 0 }), max_socket(-1)
		{
		}

		socket_wait(timeval delay) :
			m_fd({ 0 }), m_delay(delay)
		{
		}

		void reset()
		{
			FD_ZERO(&m_fd);
			max_socket = -1;
		}

		void set(socket_base &socket)
		{
			FD_SET(socket.m_socket, &m_fd);
			max_socket = std::max(max_socket, socket.m_socket);
		}

		bool isset(socket_base &socket)
		{ return FD_ISSET(socket.m_socket, &m_fd); }

		int wait()
		{
			// some impls (linux) modify delay with remaining time, so make a copy
			timeval delay = m_delay;
			return select(max_socket + 1, &m_fd, nullptr, nullptr, &delay);
		}

	private:
		fd_set m_fd;
		timeval m_delay;
		socket_handle_t max_socket;
	};
};

