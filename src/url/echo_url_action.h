#ifndef ECHO_URL_ACTION_H
#define ECHO_URL_ACTION_H
#include "abstract_url_action.h"
#include "../response/http_response.h"

class EchoUrlAction : public AbstractUrlAction {
public:
    explicit EchoUrlAction(const std::string &resource_name)
      : AbstractUrlAction(resource_name) {
    }

    [[nodiscard]] HttpResponse execute(const HttpRequest &http_request) const override {
        const std::string message = "OK";
        return HttpResponse(message, 200, "text/plain", http_request.request_param.length(), http_request.request_param);
    }
};


#endif //ECHO_URL_ACTION_H
