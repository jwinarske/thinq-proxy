#pragma once

#include "device.hpp"
#include "error.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace thinq_proxy {

/**
 * @brief Matter bridge for exposing ThinQ devices
 * 
 * Coordinates between ThinQ API and Matter protocol
 */
class MatterBridge {
public:
    MatterBridge();
    ~MatterBridge();
    
    // Delete copy operations
    MatterBridge(const MatterBridge&) = delete;
    MatterBridge& operator=(const MatterBridge&) = delete;
    
    /**
     * @brief Initialize the Matter bridge
     */
    Result<void> initialize();
    
    /**
     * @brief Add a ThinQ device to the bridge
     * @param device Device to add
     */
    Result<void> add_device(DevicePtr device);
    
    /**
     * @brief Start the Matter bridge (commissioning, advertising)
     */
    Result<void> start();
    
    /**
     * @brief Sync state between ThinQ and Matter
     */
    Result<void> sync_state();
    
    /**
     * @brief Stop the Matter bridge
     */
    void stop();
    
    /**
     * @brief Check if bridge is running
     */
    bool is_running() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace thinq_proxy
