#ifndef USER_AGENT_URL_ACTION_H
#define USER_AGENT_URL_ACTION_H
#include "abstract_url_action.h"
#include "../response/http_response.h"

class UserAgentAction : public AbstractUrlAction {
public:
    explicit UserAgentAction(const std::string &resource_name)
      : AbstractUrlAction(resource_name) {
    }

    [[nodiscard]] HttpResponse execute(const HttpRequest &http_request) const override {
        const std::string message = "OK";
        std::string body = http_request.headers.at("User-Agent");
        return HttpResponse(message, 200, "text/plain", body.length(), body, http_request.headers);
    }
};

#endif //USER_AGENT_URL_ACTION_H
