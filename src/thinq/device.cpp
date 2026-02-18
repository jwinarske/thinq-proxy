#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

Device::Device(std::string device_id, std::string alias, DeviceType type)
    : device_id_(std::move(device_id))
    , alias_(std::move(alias))
    , type_(type) {
}

} // namespace thinq_proxy
