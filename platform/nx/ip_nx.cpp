#include "ip_nx.h"

#include <nn/nifm.h>
#include <nn/nifm/nifm_ApiIpAddress.h>

#include <nn/socket.h>

void IP_NX::_resolve_hostname(List<IPAddress> &r_addresses, const String &p_hostname, IP::Type p_type) const {
    
    // block until there is network available
    nn::nifm::SubmitNetworkRequestAndWait();
    if (nn::nifm::IsNetworkAvailable()) {
        nn::socket::HostEnt* pHostEntry = nullptr;
        pHostEntry = nn::socket::GetHostEntByName(p_hostname.utf8().ptr());
        nn::socket::InAddr** pInternetAddressList = reinterpret_cast<nn::socket::InAddr **>(pHostEntry->h_addr_list);
        if (pInternetAddressList != nullptr) {
            for(int index = 0; pInternetAddressList[index] != nullptr; index++)
            {
                String addressString(nn::socket::InetNtoa(*pInternetAddressList[index]));
                IPAddress currentAddress(addressString);
                if (currentAddress.is_valid()) {
                    if (currentAddress.is_ipv4() && (p_type == IP::TYPE_IPV4 || p_type == IP::TYPE_ANY))
                        r_addresses.push_back(currentAddress);
                    if (currentAddress.is_wildcard() && p_type == IP::TYPE_NONE)
                        r_addresses.push_back(currentAddress);
                    if (p_type == IP::TYPE_IPV6)
                        r_addresses.push_back(currentAddress);
                }
            };
        }
    }
}

void IP_NX::get_local_interfaces(HashMap<String, Interface_Info> *r_interfaces) const {
    nn::socket::InAddr localIPAddress = { 0 };
    // block until there is network available
    nn::nifm::SubmitNetworkRequestAndWait();
    if (nn::nifm::IsNetworkAvailable()) {
        auto result = nn::nifm::GetCurrentPrimaryIpAddress(&localIPAddress);
        if (result.IsSuccess()) {
            Interface_Info info;
            info.name = "Default";
            info.name_friendly = "Default";
            info.index = "0";
            String addressString(nn::socket::InetNtoa(localIPAddress));
            info.ip_addresses.push_front(IPAddress(addressString));
            r_interfaces->insert(info.name, info);
        }
    }
}

void IP_NX::make_default() {
    _create = _create_nx;
}

IP *IP_NX::_create_nx() {
    return memnew(IP_NX);
}

IP_NX::IP_NX() {

}