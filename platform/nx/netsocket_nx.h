#ifndef NETSOCKET_NX_H
#define NETSOCKET_NX_H

#include "core/io/net_socket.h"
#include "core/error/error_list.h"
#include <nn/socket/socket_Api.h>

class NetSocket_NX : public NetSocket {
public:
    static void make_default();

    NetSocket_NX();
    ~NetSocket_NX();

    Error open(Type p_type, IP::Type &ip_type) override;
	virtual void close() override;
	Error bind(IPAddress p_addr, uint16_t p_port) override;
	Error listen(int p_max_pending) override;
	Error connect_to_host(IPAddress p_addr, uint16_t p_port) override;
	Error poll(PollType p_type, int timeout) const override;
	Error recv(uint8_t *p_buffer, int p_len, int &r_read) override;
	Error recvfrom(uint8_t *p_buffer, int p_len, int &r_read, IPAddress &r_ip, uint16_t &r_port, bool p_peak = false) override;
	Error send(const uint8_t *p_buffer, int p_len, int &r_sent) override;
	Error sendto(const uint8_t *p_buffer, int p_len, int &r_sent, IPAddress p_ip, uint16_t p_port) override;
	Ref<NetSocket> accept(IPAddress &r_ip, uint16_t &r_port) override;

	bool is_open() const override;
	int get_available_bytes() const override;
    Error get_socket_address(IPAddress *r_ip, uint16_t *r_port) const override;

	virtual Error set_broadcasting_enabled(bool p_enabled) override;
	void set_blocking_enabled(bool p_enabled) override;
	void set_ipv6_only_enabled(bool p_enabled) override;
	void set_tcp_no_delay_enabled(bool p_enabled) override;
	void set_reuse_address_enabled(bool p_enabled) override;
	virtual Error join_multicast_group(const IPAddress &p_multi_address, const String &p_if_name) override;
	virtual Error leave_multicast_group(const IPAddress &p_multi_address, const String &p_if_name) override;

protected:
	static NetSocket *_create_func();

private:
	enum NetError {
		ERR_NET_WOULD_BLOCK,
		ERR_NET_IS_CONNECTED,
		ERR_NET_IN_PROGRESS,
		ERR_NET_OTHER
	};

    bool _can_use_ip(const IPAddress &p_ip, const bool p_for_bind) const;
    size_t _set_addr_storage(struct nn::socket::SockAddrIn &p_addr, const IPAddress &p_ip, uint16_t p_port, IP::Type p_ip_type);
    void _set_ip_port(struct nn::socket::SockAddr &p_addr, IPAddress *r_ip, uint16_t *r_port) const;
	NetError _get_socket_error() const;
	void _set_socket(int p_sock, IP::Type p_ip_type, bool p_is_stream);
	_FORCE_INLINE_ Error _change_multicast_group(IPAddress p_ip, String p_if_name, bool p_add);

    int m_socketDescriptor;
    IP::Type m_ip_type;
	bool m_is_stream;

};

#endif // NETSOCKET_NX_H