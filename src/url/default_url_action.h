#ifndef DEFAULT_URL_ACTION_H
#define DEFAULT_URL_ACTION_H
#include <string>

#include "abstract_url_action.h"
#include "../response/http_response.h"

struct HttpRequest;

class DefaultUrlAction : public AbstractUrlAction {
public:
    explicit DefaultUrlAction(const std::string &resource_name)
      : AbstractUrlAction(resource_name) {
    }

    [[nodiscard]] std::string execute(const HttpRequest &http_request) const override {
        const std::string message = "OK";
        HttpResponse http_response = HttpResponse(message, 200, "text/plain", 0, "");
        return http_response.sendResponse();
    }
};

#endif //DEFAULT_URL_ACTION_H
