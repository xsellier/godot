#ifndef IP_NX_H
#define IP_NX_H

#include "core/io/ip.h"

class IP_NX : public IP {
    GDCLASS(IP_NX, IP);

    virtual void _resolve_hostname(List<IPAddress> &r_addresses, const String &p_hostname, IP::Type p_type) const override;

	static IP *_create_nx();

public:
	virtual void get_local_interfaces(HashMap<String, Interface_Info> *r_interfaces) const override;

	static void make_default();
	IP_NX();
};

#endif // IP_NX_H