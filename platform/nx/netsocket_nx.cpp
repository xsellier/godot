#include "netsocket_nx.h"
#include <nn/nifm.h>

#define SOCK_EMPTY -1

void NetSocket_NX::make_default() {
    _create = _create_func;
}

NetSocket_NX::NetSocket_NX() 
	: m_socketDescriptor(SOCK_EMPTY)
	, m_ip_type(IP::TYPE_NONE)
	, m_is_stream(false)
{

}

NetSocket_NX::~NetSocket_NX() {
    close();
}

Error NetSocket_NX::open(NetSocket::Type p_type, IP::Type &ip_type) {
	ERR_FAIL_COND_V(is_open(), ERR_ALREADY_IN_USE);
	ERR_FAIL_COND_V(ip_type > IP::TYPE_ANY || ip_type < IP::TYPE_NONE, ERR_INVALID_PARAMETER);

	nn::nifm::SubmitNetworkRequestAndWait();
    if (!nn::nifm::IsNetworkAvailable()) {
		print_verbose("NX Network connection unavailable.");
		return FAILED;
	}

    if (ip_type == IP::TYPE_ANY)
		ip_type = IP::TYPE_IPV4;

    nn::socket::Family family = IP::TYPE_IPV4 ? nn::socket::Family::Af_Inet : nn::socket::Family::Af_Inet6;
    nn::socket::Protocol protocol = p_type == NetSocket::TYPE_TCP ? nn::socket::Protocol::IpProto_Tcp : nn::socket::Protocol::IpProto_Udp;
    nn::socket::Type type = p_type == NetSocket::TYPE_TCP ? nn::socket::Type::Sock_Stream : nn::socket::Type::Sock_Dgram;

    m_socketDescriptor = nn::socket::Socket(family, type, protocol);

    if (m_socketDescriptor == SOCK_EMPTY && ip_type == IP::TYPE_ANY) {
		// Careful here, changing the referenced parameter so the caller knows that we are using an IPv4 socket
		// in place of a dual stack one, and further calls to _set_sock_addr will work as expected.
		ip_type = IP::TYPE_IPV4;
		family = nn::socket::Family::Af_Inet;
		m_socketDescriptor = nn::socket::Socket(family, type, protocol);
	}

    ERR_FAIL_COND_V(m_socketDescriptor == SOCK_EMPTY, FAILED);
	m_ip_type = ip_type;

    if (family == nn::socket::Family::Af_Inet6) {
		// Select IPv4 over IPv6 mapping
		set_ipv6_only_enabled(ip_type != IP::TYPE_ANY);
	}

	if (protocol == nn::socket::Protocol::IpProto_Udp) {
		// Make sure to disable broadcasting for UDP sockets.
		// Depending on the OS, this option might or might not be enabled by default. Let's normalize it.
		set_broadcasting_enabled(false);
	}

	m_is_stream = p_type == TYPE_TCP;

    return OK;
}

void NetSocket_NX::close() {
    if (m_socketDescriptor != SOCK_EMPTY)
        nn::socket::Close(m_socketDescriptor);
    m_socketDescriptor = SOCK_EMPTY;
    m_ip_type = IP::TYPE_NONE;
    m_is_stream = false;
}

Error NetSocket_NX::bind(IPAddress p_addr, uint16_t p_port) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(!_can_use_ip(p_addr, true), ERR_INVALID_PARAMETER);

    nn::socket::SockAddrIn socketAddress = { 0 };
    size_t size = _set_addr_storage(socketAddress, p_addr, p_port, m_ip_type);

	if (nn::socket::Bind(m_socketDescriptor, reinterpret_cast<nn::socket::SockAddr *>(&socketAddress), size) != 0) {
	 	_get_socket_error();
	 	print_verbose("Failed to bind socket.");
	 	close();
	 	return ERR_UNAVAILABLE;
	}

	return OK;
}

Error NetSocket_NX::listen(int p_max_pending) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);

	if (nn::socket::Listen(m_socketDescriptor, p_max_pending) != 0) {
		_get_socket_error();
		print_verbose("Failed to listen from socket.");
		close();
		return FAILED;
	};

	return OK;
}

Error NetSocket_NX::connect_to_host(IPAddress p_addr, uint16_t p_port) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(!_can_use_ip(p_addr, false), ERR_INVALID_PARAMETER);

	nn::nifm::SubmitNetworkRequestAndWait();
    if (!nn::nifm::IsNetworkAvailable()) {
		print_verbose("NX Network connection unavailable.");
		return FAILED;
	}

    nn::socket::SockAddrIn socketAddress = { 0 };
    size_t size = _set_addr_storage(socketAddress, p_addr, p_port, m_ip_type);

	if (nn::socket::Connect(m_socketDescriptor, reinterpret_cast<nn::socket::SockAddr *>(&socketAddress), size) != 0) {

		NetError err = _get_socket_error();

		switch (err) {
			// We are already connected
			case ERR_NET_IS_CONNECTED:
				return OK;
			// Still waiting to connect, try again in a while
			case ERR_NET_WOULD_BLOCK:
			case ERR_NET_IN_PROGRESS:
				return ERR_BUSY;
			default:
				print_verbose("Connection to remote host failed!");
				close();
				return FAILED;
		}
	}

	return OK;
}

Error NetSocket_NX::poll(PollType p_type, int timeout) const {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);
	nn::socket::PollFd pfd;
	pfd.fd = m_socketDescriptor;
	pfd.events = nn::socket::PollEvent::PollIn;
	pfd.revents = nn::socket::PollEvent::PollNone;

	switch (p_type) {
		case POLL_TYPE_IN:
			pfd.events = nn::socket::PollEvent::PollIn;;
			break;
		case POLL_TYPE_OUT:
			pfd.events = nn::socket::PollEvent::PollOut;
			break;
		case POLL_TYPE_IN_OUT:
			pfd.events = nn::socket::PollEvent::PollIn | nn::socket::PollEvent::PollOut;
	}

	int ret = nn::socket::Poll(&pfd, 1, timeout);
	if (ret < 0 || int16_t(pfd.revents & nn::socket::PollEvent::PollErr)) {
		_get_socket_error();
		print_verbose("Error when polling socket.");
		return FAILED;
	}

	if (ret == 0)
		return ERR_BUSY;

	return OK;
}

Error NetSocket_NX::recv(uint8_t *p_buffer, int p_len, int &r_read) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);

	r_read = nn::socket::Recv(m_socketDescriptor, p_buffer, p_len, nn::socket::MsgFlag::Msg_None);

	if (r_read < 0) {
		NetError err = _get_socket_error();
		if (err == ERR_NET_WOULD_BLOCK)
			return ERR_BUSY;

		return FAILED;
	}

	return OK;
}

Error NetSocket_NX::recvfrom(uint8_t *p_buffer, int p_len, int &r_read, IPAddress &r_ip, uint16_t &r_port, bool p_peak) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);
	nn::socket::SockAddrIn from = {};
	nn::socket::SockLenT len = sizeof(from);
    ssize_t bytesRead = 0;

	bytesRead = nn::socket::RecvFrom(m_socketDescriptor, p_buffer, p_len,  nn::socket::MsgFlag::Msg_None, reinterpret_cast<nn::socket::SockAddr *>(&from), &len);
	if (bytesRead < 0) {
		NetError err = _get_socket_error();
		if (err == ERR_NET_WOULD_BLOCK)
			return ERR_BUSY;

		return FAILED;
	}

	if (from.sin_family == nn::socket::Family::Af_Inet) {
		r_ip.set_ipv4((uint8_t *)&from.sin_addr.S_addr);
		r_port = nn::socket::InetNtohs(from.sin_port);
	} else {
		// Unsupported socket family, IPv6
		ERR_FAIL_V(FAILED);
	}

	return OK;
}

Error NetSocket_NX::send(const uint8_t *p_buffer, int p_len, int &r_sent) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);

	nn::socket::MsgFlag flags = nn::socket::MsgFlag::Msg_None;
#ifdef MSG_NOSIGNAL
	if (m_is_stream)
		flags = nn::socket::Msg_NoSignal;
#endif
	r_sent = nn::socket::Send(m_socketDescriptor, p_buffer, p_len, flags);

	if (r_sent < 0) {
		NetError err = _get_socket_error();
		if (err == ERR_NET_WOULD_BLOCK)
			return ERR_BUSY;

		return FAILED;
	}

	return OK;
}

Error NetSocket_NX::sendto(const uint8_t *p_buffer, int p_len, int &r_sent, IPAddress p_ip, uint16_t p_port) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);

	nn::socket::SockAddrIn clientAddr = {};
    nn::socket::SockLenT clientAddrLen = sizeof(clientAddr);

	r_sent = nn::socket::SendTo(m_socketDescriptor, p_buffer, p_len, nn::socket::MsgFlag::Msg_None, reinterpret_cast<nn::socket::SockAddr *>(&clientAddr), clientAddrLen);

	if (r_sent < 0) {
		NetError err = _get_socket_error();
		if (err == ERR_NET_WOULD_BLOCK)
			return ERR_BUSY;

		return FAILED;
	}

	return OK;
}

Ref<NetSocket> NetSocket_NX::accept(IPAddress &r_ip, uint16_t &r_port) {
Ref<NetSocket> out;
	ERR_FAIL_COND_V(!is_open(), out);

	nn::nifm::SubmitNetworkRequestAndWait();
    if (!nn::nifm::IsNetworkAvailable()) {
		print_verbose("NX Network connection unavailable.");
		return out;
	}

	nn::socket::SockAddrIn saClientAddress = { 0 };
    nn::socket::SockLenT clientAddressSize = sizeof(saClientAddress);
	int fd = nn::socket::Accept(m_socketDescriptor, reinterpret_cast<nn::socket::SockAddr *>(&saClientAddress), &clientAddressSize);
	if (fd == SOCK_EMPTY) {
		_get_socket_error();
		print_verbose("Error when accepting socket connection.");
		return out;
	}

	r_ip.set_ipv4((uint8_t *)&(saClientAddress.sin_addr.S_addr));
	r_port = nn::socket::InetNtohs(saClientAddress.sin_port);

	NetSocket_NX *ns = memnew(NetSocket_NX);
	ns->_set_socket(fd, m_ip_type, m_is_stream);
	ns->set_blocking_enabled(false);
	return Ref<NetSocket>(ns);
}

bool NetSocket_NX::is_open() const {
	return m_socketDescriptor != SOCK_EMPTY;
}

int NetSocket_NX::get_available_bytes() const {
	ERR_FAIL_COND_V(!is_open(), -1);

	unsigned long len;
	int ret = nn::socket::Ioctl(m_socketDescriptor, nn::socket::IoctlCommand::FionRead, &len, sizeof(len));
	if (ret == -1) {
		_get_socket_error();
		print_verbose("Error when checking available bytes on socket.");
		return -1;
	}
	return len;
}

Error NetSocket_NX::get_socket_address(IPAddress *r_ip, uint16_t *r_port) const {
    ERR_FAIL_COND_V(!is_open(), FAILED);

	nn::socket::SockAddr sa = { 0 };
    nn::socket::SockLenT saLen = sizeof(sa);
	if (nn::socket::GetSockName(m_socketDescriptor, &sa, &saLen) != 0) {
		_get_socket_error();
		print_verbose("Error when reading local socket address.");
		return FAILED;
	}
	_set_ip_port(sa, r_ip, r_port);
	return OK;
}

Error NetSocket_NX::set_broadcasting_enabled(bool p_enabled) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);
	// IPv6 has no broadcast support.
	if (m_ip_type == IP::TYPE_IPV6)
		return ERR_UNAVAILABLE;

	int par = p_enabled ? 1 : 0;
	if (nn::socket::SetSockOpt(m_socketDescriptor,  nn::socket::Level::Sol_Socket, nn::socket::Option::So_Broadcast, &par, sizeof(int)) != 0) {
		WARN_PRINT("Unable to change broadcast setting");
		return FAILED;
	}
	return OK;
}

void NetSocket_NX::set_blocking_enabled(bool p_enabled) {
	ERR_FAIL_COND(!is_open());

	int ret = 0;
	int opts = nn::socket::Fcntl(m_socketDescriptor, nn::socket::FcntlCommand::F_GetFl);
	if (p_enabled)
		ret = nn::socket::Fcntl(m_socketDescriptor, nn::socket::FcntlCommand::F_SetFl , nn::socket::FcntlFlag(opts) & ~nn::socket::FcntlFlag::O_NonBlock);
	else
		ret = nn::socket::Fcntl(m_socketDescriptor, nn::socket::FcntlCommand::F_SetFl , nn::socket::FcntlFlag(opts) | nn::socket::FcntlFlag::O_NonBlock);

	if (ret != 0)
		WARN_PRINT("Unable to change non-block mode");
}

void NetSocket_NX::set_ipv6_only_enabled(bool p_enabled) {
	// This isn't suported anyway..
	ERR_FAIL_COND(!is_open());
	// This option is only available in IPv6 sockets.
	ERR_FAIL_COND(m_ip_type == IP::TYPE_IPV4);
}

void NetSocket_NX::set_tcp_no_delay_enabled(bool p_enabled) {
	ERR_FAIL_COND(!is_open());
	ERR_FAIL_COND(!m_is_stream); // Not TCP

	int par = p_enabled ? 1 : 0;
	if (nn::socket::SetSockOpt(m_socketDescriptor, nn::socket::Level::Sol_Tcp, nn::socket::Option::Tcp_NoDelay, &par, sizeof(int)) < 0) {
		ERR_PRINT("Unable to set TCP no delay option");
	}
}

void NetSocket_NX::set_reuse_address_enabled(bool p_enabled) {
	ERR_FAIL_COND(!is_open());

	int par = p_enabled ? 1 : 0;
	if (nn::socket::SetSockOpt(m_socketDescriptor, nn::socket::Level::Sol_Socket, nn::socket::Option::So_ReuseAddr, &par, sizeof(int)) < 0) {
		WARN_PRINT("Unable to set socket REUSEADDR option!");
	}
}

Error NetSocket_NX::join_multicast_group(const IPAddress &p_multi_address, const String &p_if_name) {
	return _change_multicast_group(p_multi_address, p_if_name, true);
}
	
Error NetSocket_NX::leave_multicast_group(const IPAddress &p_multi_address, const String &p_if_name) {
	return _change_multicast_group(p_multi_address, p_if_name, false);
}

NetSocket *NetSocket_NX::_create_func() {
    return memnew(NetSocket_NX);
}

bool NetSocket_NX::_can_use_ip(const IPAddress &p_ip, const bool p_for_bind) const {

	if (p_for_bind && !(p_ip.is_valid() || p_ip.is_wildcard())) {
		return false;
	} else if (!p_for_bind && !p_ip.is_valid()) {
		return false;
	}
	// Check if socket support this IP type.
	IP::Type type = p_ip.is_ipv4() ? IP::TYPE_IPV4 : IP::TYPE_IPV6;
	return !(m_ip_type != IP::TYPE_ANY && !p_ip.is_wildcard() && m_ip_type != type);
}

size_t NetSocket_NX::_set_addr_storage(struct nn::socket::SockAddrIn &p_addr, const IPAddress &p_ip, uint16_t p_port, IP::Type p_ip_type) {
	if (p_ip_type == IP::TYPE_IPV6 || p_ip_type == IP::TYPE_ANY) { // IPv6 socket

		// IPv6 only socket with IPv4 address
		ERR_FAIL_COND_V(!p_ip.is_wildcard() && p_ip_type == IP::TYPE_IPV6 && p_ip.is_ipv4(), 0);

        return 0;
	} else { // IPv4 socket

		// IPv4 socket with IPv6 address
		ERR_FAIL_COND_V(!p_ip.is_wildcard() && !p_ip.is_ipv4(), 0);

		p_addr.sin_family = nn::socket::Family::Af_Inet;
		p_addr.sin_port = nn::socket::InetHtons(p_port); // short, network byte order

		if (p_ip.is_valid()) {
			memcpy(&p_addr.sin_addr.S_addr, p_ip.get_ipv4(), 4);
		} else {
			p_addr.sin_addr.S_addr = nn::socket::InetHtonl(nn::socket::InAddr_Any);
		}

		return sizeof(p_addr);
	}
}

void NetSocket_NX::_set_ip_port(struct nn::socket::SockAddr &p_addr, IPAddress *r_ip, uint16_t *r_port) const {
    if (p_addr.sa_family == nn::socket::Family::Af_Inet) {
        struct nn::socket::SockAddrIn *addr4 = reinterpret_cast<nn::socket::SockAddrIn *>(&p_addr);
        if (r_ip) {
			r_ip->set_ipv4((uint8_t *)&(addr4->sin_addr.S_addr));
		}
		if (r_port) {
			*r_port = nn::socket::InetNtohs(addr4->sin_port);
		}
    } 
    // If it was ipv6 you are out of luck :-(
}

NetSocket_NX::NetError NetSocket_NX::_get_socket_error() const {
	nn::socket::Errno error = nn::socket::GetLastError();
	if (error == nn::socket::Errno::EIsConn)
		return ERR_NET_IS_CONNECTED;
	if (error == nn::socket::Errno::EInProgress || error == nn::socket::Errno::EAlready)
		return ERR_NET_IN_PROGRESS;
	if (error == nn::socket::Errno::EAgain || error == nn::socket::Errno::EWouldBlock)
		return ERR_NET_WOULD_BLOCK;
	print_verbose("Socket error: " + itos(uint32_t(error)));
	return ERR_NET_OTHER;

}

void NetSocket_NX::_set_socket(int p_sock, IP::Type p_ip_type, bool p_is_stream) {
	m_socketDescriptor = p_sock;
	m_ip_type = p_ip_type;
	m_is_stream = p_is_stream;
}

_FORCE_INLINE_ Error NetSocket_NX::_change_multicast_group(IPAddress p_ip, String p_if_name, bool p_add) {
	ERR_FAIL_COND_V(!is_open(), ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(!_can_use_ip(p_ip, false), ERR_INVALID_PARAMETER);

	int ret = -1;

	IPAddress if_ip;
	HashMap<String, IP::Interface_Info> if_info;
	IP::get_singleton()->get_local_interfaces(&if_info);
	for (KeyValue<String, IP::Interface_Info> &E : if_info) {
		IP::Interface_Info &c = E.value;
		if (c.name != p_if_name)
			continue;

		for (const IPAddress &F : c.ip_addresses) {
			if (!F.is_ipv4())
				continue; // Wrong IP type
			if_ip = F;
			break;
		}
		break;
	}

	ERR_FAIL_COND_V(!if_ip.is_valid(), ERR_INVALID_PARAMETER);
	nn::socket::IpMreq greq;
	nn::socket::Option sock_opt = p_add ? nn::socket::Option::Ip_Add_Membership : nn::socket::Option::Ip_Drop_Membership;
	memcpy(&greq.imr_multiaddr, p_ip.get_ipv4(), 4);
	memcpy(&greq.imr_interface, if_ip.get_ipv4(), 4);
	ret = nn::socket::SetSockOpt(m_socketDescriptor, nn::socket::Level::Sol_Ip, sock_opt, (const char *)&greq, sizeof(greq));

	ERR_FAIL_COND_V(ret != 0, FAILED);

	return OK;
}