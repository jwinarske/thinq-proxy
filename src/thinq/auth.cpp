#include "thinq_proxy/auth.hpp"
#include <format>

namespace thinq_proxy {

Auth::Auth(std::string pat_token, Country country, std::string client_id)
    : pat_token_(std::move(pat_token))
    , country_(country)
    , client_id_(std::move(client_id)) {
}

std::string Auth::get_auth_header() const {
    return std::format("Bearer {}", pat_token_);
}

Result<bool> Auth::validate() const {
    if (pat_token_.empty()) {
        return std::unexpected(make_error_code(ErrorCode::AuthenticationFailed));
    }
    if (client_id_.empty()) {
        return std::unexpected(make_error_code(ErrorCode::InvalidConfiguration));
    }
    return true;
}

} // namespace thinq_proxy
